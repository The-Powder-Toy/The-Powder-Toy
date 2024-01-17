#include "LuaSocket.h"
#include "LuaScriptInterface.h"
#include "Misc.h"
#include <stdint.h>
#include <algorithm>

namespace LuaSocket
{
	static int GetTime(lua_State *l)
	{
		lua_pushnumber(l, Now());
		return 1;
	}

	static int Sleep(lua_State *l)
	{
		Timeout(luaL_checknumber(l, 1));
		return 0;
	}

	void Open(lua_State *l)
	{
		lua_newtable(l);
		struct luaL_Reg socketMethods[] = {
			{   "sleep", LuaSocket::Sleep   },
			{ "getTime", LuaSocket::GetTime },
			{      NULL, NULL                  },
		};
		luaL_register(l, NULL, socketMethods);
		lua_setglobal(l, "socket");
		OpenTCP(l);
	}
}
