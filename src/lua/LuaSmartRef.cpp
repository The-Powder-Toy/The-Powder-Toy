#ifdef LUACONSOLE
#include "LuaSmartRef.h"

void LuaSmartRef::Clear()
{
	luaL_unref(rootl, LUA_REGISTRYINDEX, ref);
	ref = LUA_REFNIL;
}

LuaSmartRef::LuaSmartRef(lua_State *l) :
	ref(LUA_REFNIL)
{
	tpt_lua_getmainthread(l);
	rootl = lua_tothread(l, -1);
	lua_pop(l, 1);
}

LuaSmartRef::~LuaSmartRef()
{
	Clear();
}

void LuaSmartRef::Assign(lua_State *l, int index)
{
	if (index < 0)
	{
		index = lua_gettop(l) + index + 1;
	}
	Clear();
	lua_pushvalue(l, index);
	ref = luaL_ref(l, LUA_REGISTRYINDEX);
}

int LuaSmartRef::Push(lua_State *l)
{
	lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
	return lua_type(l, -1);
}

#endif
