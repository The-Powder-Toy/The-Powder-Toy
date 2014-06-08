#ifndef LUAINC_H
#define LUAINC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LUA_INC
#include "lua5.1/lua.h"
#include "lua5.1/lauxlib.h"
#include "lua5.1/lualib.h"
#else
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
