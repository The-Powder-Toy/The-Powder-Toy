#include "LuaSmartRef.h"
#include "LuaScriptInterface.h"

void LuaSmartRef::Clear()
{
	auto *lsi = GetLSI();
	if (lsi)
	{
		luaL_unref(lsi->L, LUA_REGISTRYINDEX, ref);
		ref = LUA_REFNIL;
	}
}

LuaSmartRef::~LuaSmartRef()
{
	Clear();
}

void LuaSmartRef::Assign(lua_State *L, int index)
{
	if (index < 0)
	{
		index = lua_gettop(L) + index + 1;
	}
	Clear();
	lua_pushvalue(L, index);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
}

int LuaSmartRef::Push(lua_State *L)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	return lua_type(L, -1);
}
