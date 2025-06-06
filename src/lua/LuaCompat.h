#pragma once

extern "C"
{

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if LUA_VERSION_NUM >= 502
void luaL_register(lua_State *L, const char *libname, const luaL_Reg *l);
#define lua_strlen(L,i) lua_rawlen(L, (i))
#define lua_objlen(L,i) lua_rawlen(L, (i))
#define lua_equal(L,idx1,idx2) lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#define lua_lessthan(L,idx1,idx2) lua_compare(L,(idx1),(idx2),LUA_OPLT)

LUALIB_API int (luaL_typerror) (lua_State *L, int narg, const char *tname);
#else
# ifndef lua_pushglobaltable // * Thank you moonjit
LUALIB_API void (lua_pushglobaltable) (lua_State *L);
# endif
#endif

}
