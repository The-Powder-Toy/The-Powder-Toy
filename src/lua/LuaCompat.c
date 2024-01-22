#include "LuaCompat.h"

#if LUA_VERSION_NUM >= 502
// Implement missing luaL_typerror function
int luaL_typerror(lua_State *L, int narg, const char *tname)
{
	const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}

void luaL_register(lua_State *L, const char *libname, const luaL_Reg *l)
{
	if (libname)
	{
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, libname);
	}
	luaL_setfuncs(L, l, 0);
}
#else
# ifndef lua_pushglobaltable // * Thank you moonjit
// Implement function added in lua 5.2 that we now use
void lua_pushglobaltable(lua_State *L)
{
	lua_pushvalue(L, LUA_GLOBALSINDEX);
}
# endif
#endif
