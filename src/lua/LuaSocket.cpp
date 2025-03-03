#include "LuaScriptInterface.h"
#include "Misc.h"

int LuaSocket::GetTime(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent();
	lua_pushnumber(L, LuaSocket::Now());
	return 1;
}

int LuaSocket::Sleep(lua_State *L)
{
	GetLSI()->AssertInterfaceEvent();
	LuaSocket::Timeout(luaL_checknumber(L, 1));
	return 0;
}

void LuaSocket::Open(lua_State *L)
{
	static const luaL_Reg reg[] = {
		{ "sleep", LuaSocket::Sleep },
		{ "getTime", LuaSocket::GetTime },
		{ nullptr, nullptr }
	};
	lua_newtable(L);
	luaL_register(L, nullptr, reg);
	lua_setglobal(L, "socket");
	OpenTCP(L);
}
