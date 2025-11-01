#include "LuaScriptInterface.h"
#include "client/http/requestmanager/RequestManager.h"
#include "common/Defer.h"
#include <emscripten.h>

struct WebSocket
{
	LuaSmartRef onOpen;
	LuaSmartRef onError;
	LuaSmartRef onClose;
	LuaSmartRef onMessage;
	int id = -1;
};

EMSCRIPTEN_KEEPALIVE extern "C" void RequestManager_WebsocketOnOpenThunk(WebSocket *ws)
{
	if (!ws->onOpen)
	{
		return;
	}
	auto *lsi = GetLSI();
	lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, ws->onOpen);
	if (tpt_lua_pcall(lsi->L, 0, 0, 0, eventTraitInterface))
	{
		lsi->Log(CommandInterface::LogError, LuaGetError());
		lua_pop(lsi->L, 1);
	}
}

EMSCRIPTEN_KEEPALIVE extern "C" void RequestManager_WebsocketOnErrorThunk(WebSocket *ws)
{
	if (!ws->onError)
	{
		return;
	}
	auto *lsi = GetLSI();
	lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, ws->onError);
	if (tpt_lua_pcall(lsi->L, 0, 0, 0, eventTraitInterface))
	{
		lsi->Log(CommandInterface::LogError, LuaGetError());
		lua_pop(lsi->L, 1);
	}
}

EMSCRIPTEN_KEEPALIVE extern "C" void RequestManager_WebsocketOnCloseThunk(WebSocket *ws)
{
	if (!ws->onClose)
	{
		return;
	}
	auto *lsi = GetLSI();
	lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, ws->onClose);
	lua_pushinteger(lsi->L, EM_ASM_INT({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return websocket.event.code;
	}, ws->id));
	auto *reason = (char *)EM_ASM_PTR({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return stringToNewUTF8(websocket.event.reason);
	}, ws->id);
	Defer freeReason([reason]() {
		free(reason);
	});
	lua_pushstring(lsi->L, reason);
	lua_pushboolean(lsi->L, EM_ASM_INT({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return websocket.event.wasClean;
	}, ws->id));
	if (tpt_lua_pcall(lsi->L, 3, 0, 0, eventTraitInterface))
	{
		lsi->Log(CommandInterface::LogError, LuaGetError());
		lua_pop(lsi->L, 1);
	}
}

EMSCRIPTEN_KEEPALIVE extern "C" void RequestManager_WebsocketOnMessageThunk(WebSocket *ws)
{
	if (!ws->onMessage)
	{
		return;
	}
	auto *lsi = GetLSI();
	lua_rawgeti(lsi->L, LUA_REGISTRYINDEX, ws->onMessage);
	auto binary = EM_ASM_INT({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return !!(websocket.event.data instanceof ArrayBuffer);
	}, ws->id);
	if (binary)
	{
		std::vector<char> data(EM_ASM_INT({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			return websocket.event.data.byteLength;
		}, ws->id));
		EM_ASM({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			writeArrayToMemory(new Int8Array(websocket.event.data), $1);
		}, ws->id, data.data());
		lua_pushlstring(lsi->L, data.data(), data.size());
	}
	else
	{
		auto *data = (char *)EM_ASM_PTR({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			return stringToNewUTF8(websocket.event.data);
		}, ws->id);
		Defer freeData([data]() {
			free(data);
		});
		lua_pushstring(lsi->L, data);
	}
	lua_pushboolean(lsi->L, binary);
	if (tpt_lua_pcall(lsi->L, 2, 0, 0, eventTraitInterface))
	{
		lsi->Log(CommandInterface::LogError, LuaGetError());
		lua_pop(lsi->L, 1);
	}
}

static int Close(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in WebSocket::New
	auto *ws = (WebSocket *)luaL_checkudata(L, 1, "WebSocket");
	int specified = 0;
	int code = 0;
	ByteString reason;
	if (!lua_isnoneornil(L, 2))
	{
		code = luaL_checkinteger(L, 2);
		specified |= 1;
		if (!lua_isnoneornil(L, 3))
		{
			reason = tpt_lua_checkByteString(L, 3);
			specified |= 2;
		}
	}
	EM_ASM({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		if ($1 & 2) {
			websocket.socket.close($2, UTF8ToString($3));
		} else if ($1 & 1) {
			websocket.socket.close($2);
		} else {
			websocket.socket.close();
		}
	}, ws->id, specified, code, reason.c_str());
	return 0;
}

static int Send(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in WebSocket::New
	auto *ws = (WebSocket *)luaL_checkudata(L, 1, "WebSocket");
	luaL_checktype(L, 2, LUA_TSTRING);
	size_t size;
	auto *data = lua_tolstring(L, 2, &size);
	auto binary = lua_toboolean(L, 3);
	int takeBase, takeSize;
	{
		auto first = luaL_optinteger(L, 4, 1);
		auto last = luaL_optinteger(L, 5, int(size));
		if (first < 1 || last > int(size) || last < first)
		{
			return luaL_error(L, "invalid range");
		}
		takeBase = first - 1;
		takeSize = last - takeBase;
	}
	if (binary)
	{
		EM_ASM({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			websocket.socket.send(HEAP8.slice($1, $1 + $2));
		}, ws->id, data + takeBase, takeSize);
	}
	else
	{
		ByteString str(data + takeBase, data + takeBase + takeSize);
		EM_ASM({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			websocket.socket.send(UTF8ToString($1));
		}, ws->id, str.c_str());
	}
	return 0;
}

template<LuaSmartRef WebSocket::*Member>
static int CallbackSetter(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in WebSocket::New
	auto *ws = (WebSocket *)luaL_checkudata(L, 1, "WebSocket");
	if (!lua_isnoneornil(L, 2))
	{
		luaL_checktype(L, 2, LUA_TFUNCTION);
	}
	(ws->*Member).Assign(L, 2);
	return 0;
}

static int Status(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in WebSocket::New
	auto *ws = (WebSocket *)luaL_checkudata(L, 1, "WebSocket");
	switch (EM_ASM_INT({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return websocket.socket.readyState;
	}, ws->id))
	{
		case 0: lua_pushfstring(L, "connecting"); break;
		case 1: lua_pushfstring(L, "open"      ); break;
		case 2: lua_pushfstring(L, "closing"   ); break;
		case 3: lua_pushfstring(L, "closed"    ); break;
	}
	lua_pushinteger(L, EM_ASM_INT({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		return websocket.socket.bufferedAmount;
	}, ws->id));
	return 2;
}

static int New(lua_State *L)
{
	// there's no actual architectural reason to require socket handles to be managed from interface events
	// because the HTTP thread doesn't care what thread they are managed from, but requiring this
	// makes access patterns cleaner, so we do it anyway
	GetLSI()->AssertInterfaceEvent();
	if (http::RequestManager::Ref().DisableNetwork())
	{
		return luaL_error(L, "network disabled");
	}
	ByteString url = tpt_lua_checkByteString(L, 1);
	std::vector<ByteString> protocols;
	if (!lua_isnoneornil(L, 2))
	{
		luaL_checktype(L, 2, LUA_TTABLE);
		auto size = lua_objlen(L, 2);
		for (auto i = 0U; i < size; ++i)
		{
			lua_rawgeti(L, 2, i + 1);
			if (!lua_isstring(L, -1))
			{
				luaL_error(L, "protocol %i is not a string", i + 1);
			}
			protocols.push_back(tpt_lua_toByteString(L, -1));
			lua_pop(L, 1);
		}
	}
	auto *ws = (WebSocket *)lua_newuserdata(L, sizeof(WebSocket));
	new(ws) WebSocket;
	ws->id = EM_ASM_INT({
		let id = 0;
		while (Module.emscriptenRequestManager.websockets[id])
		{
			id += 1;
		}
		let websocket = {};
		websocket.token = $0;
		websocket.protocols = [];
		Module.emscriptenRequestManager.websockets[id] = websocket;
		return id;
	}, ws);
	for (auto &protocol : protocols)
	{
		EM_ASM({
			let websocket = Module.emscriptenRequestManager.websockets[$0];
			websocket.protocols.push(UTF8ToString($1));
		}, ws->id, protocol.c_str());
	}
	EM_ASM({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		websocket.socket = new WebSocket(UTF8ToString($1), websocket.protocols);
		websocket.socket.binaryType = 'arraybuffer';
		websocket.socket.addEventListener('open', () => {
			Module.emscriptenRequestManager.websocketOnOpenThunk(websocket.token);
		});
		websocket.socket.addEventListener('error', () => {
			Module.emscriptenRequestManager.websocketOnErrorThunk(websocket.token);
		});
		websocket.socket.addEventListener('close', (event) => {
			websocket.event = event;
			Module.emscriptenRequestManager.websocketOnCloseThunk(websocket.token);
			delete websocket.event;
		});
		websocket.socket.addEventListener('message', (event) => {
			websocket.event = event;
			Module.emscriptenRequestManager.websocketOnMessageThunk(websocket.token);
			delete websocket.event;
		});
	}, ws->id, url.c_str());
	luaL_newmetatable(L, "WebSocket");
	lua_setmetatable(L, -2);
	return 1;
}

static int GC(lua_State *L)
{
	// not subject to the check in WebSocket::New; that would be disastrous, and thankfully,
	// as explained there, we're not missing out on any functionality either
	auto *ws = (WebSocket *)luaL_checkudata(L, 1, "WebSocket");
	assert(ws->id >= 0);
	EM_ASM({
		let websocket = Module.emscriptenRequestManager.websockets[$0];
		websocket.socket.close();
		Module.emscriptenRequestManager.websockets[$0] = null;
	}, ws->id);
	ws->id = -1;
	ws->~WebSocket();
	return 0;
}

namespace LuaSocket
{
	void OpenTCP(lua_State *L)
	{
		EM_ASM({
			Module.emscriptenRequestManager.websockets = [];
			Module.emscriptenRequestManager.websocketOnOpenThunk = Module.cwrap(
				'RequestManager_WebsocketOnOpenThunk',
				null,
				[ 'number' ]
			);
			Module.emscriptenRequestManager.websocketOnErrorThunk = Module.cwrap(
				'RequestManager_WebsocketOnErrorThunk',
				null,
				[ 'number' ]
			);
			Module.emscriptenRequestManager.websocketOnCloseThunk = Module.cwrap(
				'RequestManager_WebsocketOnCloseThunk',
				null,
				[ 'number' ]
			);
			Module.emscriptenRequestManager.websocketOnMessageThunk = Module.cwrap(
				'RequestManager_WebsocketOnMessageThunk',
				null,
				[ 'number' ]
			);
		});

		luaL_newmetatable(L, "WebSocket");
		lua_pushcfunction(L, GC);
		lua_setfield(L, -2, "__gc");
		lua_newtable(L);
		struct luaL_Reg webSocketIndexMethods[] = {
			{       "close", Close                                 },
			{        "send", Send                                  },
			{      "onOpen", CallbackSetter<&WebSocket::onOpen   > },
			{     "onError", CallbackSetter<&WebSocket::onError  > },
			{     "onClose", CallbackSetter<&WebSocket::onClose  > },
			{   "onMessage", CallbackSetter<&WebSocket::onMessage> },
			{      "status", Status                                },
			{       nullptr, nullptr                               },
		};
		luaL_register(L, nullptr, webSocketIndexMethods);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
		lua_getglobal(L, "socket");
		lua_pushcfunction(L, New);
		lua_setfield(L, -2, "web");
		lua_pop(L, 1);
	}
}
