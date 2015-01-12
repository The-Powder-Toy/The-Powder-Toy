#include "LuaCompat.h"

#if LUA_VERSION_NUM >= 502
//implement missing luaL_typerror function
int luaL_typerror (lua_State *L, int narg, const char *tname)
{
	const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}

#endif
