#include "LuaCompat.h"

#if LUA_VERSION_NUM >= 502
// Implement missing luaL_typerror function
int luaL_typerror (lua_State *L, int narg, const char *tname)
{
	const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}

void tpt_lua_setmainthread(lua_State *L)
{
}

void tpt_lua_getmainthread(lua_State *L)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
}

#else

// Implement function added in lua 5.2 that we now use
void lua_pushglobaltable(lua_State *L)
{
	lua_pushvalue(L, LUA_GLOBALSINDEX);
}

void tpt_lua_setmainthread(lua_State *L)
{
	lua_pushthread(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "tpt_lua_mainthread");
}

void tpt_lua_getmainthread(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, "tpt_lua_mainthread");
}

#endif

// Useful helper function, mainly used for logging
int luaL_tostring(lua_State *L, int n)
{
	luaL_checkany(L, n);
	switch (lua_type(L, n))
	{
		case LUA_TNUMBER:
			lua_pushstring(L, lua_tostring(L, n));
			break;
		case LUA_TSTRING:
			lua_pushvalue(L, n);
			break;
		case LUA_TBOOLEAN:
			lua_pushstring(L, (lua_toboolean(L, n) ? "true" : "false"));
			break;
		case LUA_TNIL:
			lua_pushliteral(L, "nil");
			break;
		default:
			lua_pushfstring(L, "%s: %p", luaL_typename(L, n), lua_topointer(L, n));
			break;
	}
	return 1;
}
