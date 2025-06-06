#include "LuaScriptInterface.h"
#include "client/http/Request.h"
#include "client/Client.h"
#include "json/json.h"
#include "LuaScriptInterface.h"
#include "Format.h"
#include "Config.h"
#include <memory>
#include <iostream>

namespace LuaHttp
{
class RequestHandle
{
public:
	enum RequestType
	{
		normal,
		getAuthToken,
	};

private:
	decltype(LuaScriptInterface::requestHandles)::iterator requestHandlesIt;
	std::unique_ptr<http::Request> request;
	bool dead = false;
	bool seenRunningThisTick = true;
	RequestType type;

	RequestHandle() = default;

	std::pair<int, ByteString> FinishGetAuthToken(ByteString data)
	{
		std::istringstream ss(data);
		Json::Value root;
		try
		{
			ss >> root;
			auto status = root["Status"].asString();
			if (status == "OK")
			{
				return { 200, root["Token"].asString() };
			}
			return { 403, status };
		}
		catch (std::exception &e)
		{
			std::cerr << "bad auth response: " << e.what() << std::endl;
		}
		return { 600, {} };
	}

public:
	static int Make(lua_State *L, const ByteString &uri, bool isPost, const ByteString &verb, RequestType type, const http::PostData &postData, const std::vector<http::Header> &headers)
	{
		// there's no actual architectural reason to require HTTP handles to be managed from interface events
		// because the HTTP thread doesn't care what thread the requests are managed from, but requiring this
		// makes access patterns cleaner, so we do it anyway
		auto *lsi = GetLSI();
		lsi->AssertInterfaceEvent();
		auto authUser = Client::Ref().GetAuthUser();
		if (type == getAuthToken && !authUser.UserID)
		{
			lua_pushnil(L);
			lua_pushliteral(L, "not authenticated");
			return 2;
		}
		auto *rh = (RequestHandle *)lua_newuserdata(L, sizeof(RequestHandle));
		if (!rh)
		{
			return 0;
		}
		new(rh) RequestHandle();
		rh->type = type;
		rh->request = std::make_unique<http::Request>(uri);
		if (verb.size())
		{
			rh->request->Verb(verb);
		}
		for (const auto &header : headers)
		{
			rh->request->AddHeader(header);
		}
		if (isPost)
		{
			rh->request->AddPostData(postData);
		}
		if (type == getAuthToken)
		{
			rh->request->AuthHeaders(ByteString::Build(authUser.UserID), authUser.SessionID);
		}
		rh->request->Start();
		luaL_newmetatable(L, "HTTPRequest");
		lua_setmetatable(L, -2);
		lsi->requestHandles.push_back(rh);
		rh->requestHandlesIt = --lsi->requestHandles.end();
		return 1;
	}

	~RequestHandle()
	{
		if (!dead)
		{
			Cancel();
		}
		auto *lsi = GetLSI();
		lsi->requestHandles.erase(requestHandlesIt);
	}

	enum class Status
	{
		running,
		done,
		dead,
	};
	Status GetStatus()
	{
		if (!seenRunningThisTick)
		{
			if (dead)
			{
				return Status::dead;
			}
			if (request->CheckDone())
			{
				return Status::done;
			}
			seenRunningThisTick = true;
		}
		return Status::running;
	}

	void ClearSeenRunningThisTick()
	{
		seenRunningThisTick = false;
	}

	std::pair<int64_t, int64_t> Progress()
	{
		int64_t total = 0;
		int64_t done = 0;
		if (!dead)
		{
			std::tie(total, done) = request->CheckProgress();
		}
		return { total, done };
	}

	void Cancel()
	{
		if (!dead)
		{
			request.reset();
			dead = true;
		}
	}

	std::pair<int, ByteString> Finish(std::vector<http::Header> &headers)
	{
		assert(GetStatus() == Status::done);
		if (type != getAuthToken)
		{
			headers = request->ResponseHeaders();
		}
		// Get this separately so it's always present.
		auto status = request->StatusCode();
		ByteString data;
		try
		{
			data = request->Finish().second;
		}
		catch (const http::RequestError &ex)
		{
			// Nothing, the only way to fail here is to fail in RequestManager, and
			// that means the problem has already been printed to std::cerr.
		}
		request.reset();
		if (type == getAuthToken)
		{
			if (status == 200)
			{
				std::tie(status, data) = FinishGetAuthToken(data);
			}
		}
		dead = true;
		return { status, data };
	}
};
}

static int HTTPRequest_gc(lua_State *L)
{
	// not subject to the check in RequestHandle::Make; that would be disastrous, and thankfully,
	// as explained there, we're not missing out on any functionality either
	auto *rh = (LuaHttp::RequestHandle *)luaL_checkudata(L, 1, "HTTPRequest");
	rh->~RequestHandle();
	return 0;
}

static int HTTPRequest_status(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in RequestHandle::Make
	auto *rh = (LuaHttp::RequestHandle *)luaL_checkudata(L, 1, "HTTPRequest");
	switch (rh->GetStatus())
	{
	case LuaHttp::RequestHandle::Status::running: lua_pushliteral(L, "running"); break;
	case LuaHttp::RequestHandle::Status::done   : lua_pushliteral(L, "done")   ; break;
	case LuaHttp::RequestHandle::Status::dead   : lua_pushliteral(L, "dead")   ; break;
	}
	return 1;
}

void LuaHttp::Tick(lua_State *)
{
	for (auto ptr : GetLSI()->requestHandles)
	{
		ptr->ClearSeenRunningThisTick();
	}
}

static int HTTPRequest_progress(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in RequestHandle::Make
	auto *rh = (LuaHttp::RequestHandle *)luaL_checkudata(L, 1, "HTTPRequest");
	if (rh->GetStatus() != LuaHttp::RequestHandle::Status::dead)
	{
		auto [ total, done ] = rh->Progress();
		lua_pushinteger(L, total);
		lua_pushinteger(L, done);
		return 2;
	}
	return 0;
}

static int HTTPRequest_cancel(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in RequestHandle::Make
	auto *rh = (LuaHttp::RequestHandle *)luaL_checkudata(L, 1, "HTTPRequest");
	if (rh->GetStatus() != LuaHttp::RequestHandle::Status::dead)
	{
		rh->Cancel();
	}
	return 0;
}

static int HTTPRequest_finish(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent(); // see the check in RequestHandle::Make
	auto *rh = (LuaHttp::RequestHandle *)luaL_checkudata(L, 1, "HTTPRequest");
	if (rh->GetStatus() == LuaHttp::RequestHandle::Status::done)
	{
		std::vector<http::Header> headers;
		auto [ status, data ] = rh->Finish(headers);
		tpt_lua_pushByteString(L, data);
		lua_pushinteger(L, status);
		lua_newtable(L);
		for (auto i = 0; i < int(headers.size()); ++i)
		{
			lua_newtable(L);
			lua_pushlstring(L, headers[i].name.data(), headers[i].name.size());
			lua_rawseti(L, -2, 1);
			lua_pushlstring(L, headers[i].value.data(), headers[i].value.size());
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i + 1);
		}
		return 3;
	}
	return 0;
}

static int request(lua_State *L, bool isPost)
{
	ByteString uri = tpt_lua_checkByteString(L, 1);
	http::PostData postData;
	auto headersIndex = 2;
	auto verbIndex = 3;

	if (isPost)
	{
		headersIndex += 1;
		verbIndex += 1;
		if (lua_isstring(L, 2))
		{
			postData = tpt_lua_toByteString(L, 2);
		}
		else if (lua_istable(L, 2))
		{
			postData = http::FormData{};
			auto &formData = std::get<http::FormData>(postData);
			auto size = lua_objlen(L, 2);
			if (size)
			{
				for (auto i = 0U; i < size; ++i)
				{
					auto &formItem = formData.emplace_back();
					lua_rawgeti(L, 2, i + 1);
					if (!lua_istable(L, -1))
					{
						luaL_error(L, "form item %i is not a table", i + 1);
					}
					{
						lua_rawgeti(L, -1, 1);
						if (!lua_isstring(L, -1))
						{
							luaL_error(L, "name of form item %i is not a string", i + 1);
						}
						formItem.name = tpt_lua_toByteString(L, -1);
						lua_pop(L, 1);
					}
					{
						lua_rawgeti(L, -1, 2);
						if (!lua_isstring(L, -1))
						{
							luaL_error(L, "value of form item %i is not a string", i + 1);
						}
						formItem.value = tpt_lua_toByteString(L, -1);
						lua_pop(L, 1);
					}
					{
						lua_rawgeti(L, -1, 3);
						if (!lua_isnoneornil(L, -1))
						{
							if (!lua_isstring(L, -1))
							{
								luaL_error(L, "filename of form item %i is not a string", i + 1);
							}
							formItem.filename = tpt_lua_toByteString(L, -1);
						}
						lua_pop(L, 1);
					}
					{
						lua_rawgeti(L, -1, 4);
						if (!lua_isnoneornil(L, -1))
						{
							if (!lua_isstring(L, -1))
							{
								luaL_error(L, "content type of form item %i is not a string", i + 1);
							}
							formItem.contentType = tpt_lua_toByteString(L, -1);
						}
						lua_pop(L, 1);
					}
					lua_pop(L, 1);
				}
			}
			else
			{
				lua_pushnil(L);
				while (lua_next(L, 2))
				{
					lua_pushvalue(L, -2);
					auto &formItem = formData.emplace_back();
					formItem.name = tpt_lua_toByteString(L, -1);
					formItem.value = tpt_lua_toByteString(L, -2);
					lua_pop(L, 2);
				}
			}
		}
	}

	std::vector<http::Header> headers;
	if (lua_istable(L, headersIndex))
	{
		auto size = lua_objlen(L, headersIndex);
		if (size)
		{
			for (auto i = 0U; i < size; ++i)
			{
				lua_rawgeti(L, headersIndex, i + 1);
				if (!lua_istable(L, -1))
				{
					luaL_error(L, "header %i is not a table", i + 1);
				}
				lua_rawgeti(L, -1, 1);
				if (!lua_isstring(L, -1))
				{
					luaL_error(L, "name of header %i is not a string", i + 1);
				}
				auto name = tpt_lua_toByteString(L, -1);
				lua_pop(L, 1);
				lua_rawgeti(L, -1, 2);
				if (!lua_isstring(L, -1))
				{
					luaL_error(L, "value of header %i is not a string", i + 1);
				}
				auto value = tpt_lua_toByteString(L, -1);
				lua_pop(L, 1);
				headers.push_back({ name, value });
				lua_pop(L, 1);
			}
		}
		else
		{
			// old dictionary format
			lua_pushnil(L);
			while (lua_next(L, headersIndex))
			{
				lua_pushvalue(L, -2);
				headers.push_back({ tpt_lua_toByteString(L, -1), tpt_lua_toByteString(L, -2) });
				lua_pop(L, 2);
			}
		}
	}

	auto verb = tpt_lua_optByteString(L, verbIndex, "");
	return LuaHttp::RequestHandle::Make(L, uri, isPost, verb, LuaHttp::RequestHandle::normal, postData, headers);
}

static int getAuthToken(lua_State *L)
{
	return LuaHttp::RequestHandle::Make(L, ByteString::Build(SERVER, "/ExternalAuth.api?Action=Get&Audience=", format::URLEncode(tpt_lua_checkByteString(L, 1))), false, {}, LuaHttp::RequestHandle::getAuthToken, {}, {});
}

static int get(lua_State *L)
{
	return request(L, false);
}

static int post(lua_State *L)
{
	return request(L, true);
}

void LuaHttp::Open(lua_State *L)
{
	{
		static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, HTTPRequest_ ## v }
			LFUNC(status),
			LFUNC(progress),
			LFUNC(cancel),
			LFUNC(finish),
#undef LFUNC
			{ nullptr, nullptr }
		};
		luaL_newmetatable(L, "HTTPRequest");
		lua_pushcfunction(L, HTTPRequest_gc);
		lua_setfield(L, -2, "__gc");
		lua_newtable(L);
		luaL_register(L, nullptr, reg);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
	{
		static const luaL_Reg reg[] = {
#define LFUNC(v) { #v, v }
			LFUNC(get),
			LFUNC(post),
			LFUNC(getAuthToken),
#undef LFUNC
			{ nullptr, nullptr }
		};
		lua_newtable(L);
		luaL_register(L, nullptr, reg);
		lua_setglobal(L, "http");
	}
}
