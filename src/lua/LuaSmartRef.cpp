#ifdef LUACONSOLE
#include "LuaSmartRef.h"

void LuaSmartRef::Clear()
{
	luaL_unref(l, LUA_REGISTRYINDEX, ref);
	ref = LUA_REFNIL;
}

LuaSmartRef::LuaSmartRef(lua_State *state) :
	ref(LUA_REFNIL),
	l(state)
{
}

LuaSmartRef::~LuaSmartRef()
{
	Clear();
}

void LuaSmartRef::Assign(int index)
{
	Clear();
	lua_pushvalue(l, index);
	ref = luaL_ref(l, LUA_REGISTRYINDEX);
}

LuaSmartRef::operator int() const
{
	return ref;
}

LuaSmartRef::operator bool() const
{
	return ref != LUA_REFNIL;
}
#endif
