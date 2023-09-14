#include "LuaSmartRef.h"
#include "LuaScriptInterface.h"

void LuaSmartRef::Clear()
{
	auto *luacon_ci = static_cast<LuaScriptInterface *>(commandInterface);
	if (luacon_ci)
	{
		luaL_unref(luacon_ci->l, LUA_REGISTRYINDEX, ref);
		ref = LUA_REFNIL;
	}
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
