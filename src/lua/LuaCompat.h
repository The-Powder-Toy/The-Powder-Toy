#ifndef LUAINC_H
#define LUAINC_H
#include "Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

LUALIB_API void tpt_lua_setmainthread(lua_State *L);
LUALIB_API void tpt_lua_getmainthread(lua_State *L);

#if LUA_VERSION_NUM >= 502
LUALIB_API int (luaL_typerror) (lua_State *L, int narg, const char *tname);
#else
LUALIB_API void (lua_pushglobaltable) (lua_State *L);
#endif
int luaL_tostring(lua_State *L, int n);

#ifdef __cplusplus
}
#endif

#endif
