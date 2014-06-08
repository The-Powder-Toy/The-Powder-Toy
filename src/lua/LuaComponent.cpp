#ifdef LUACONSOLE

#include <iostream>
#include "LuaComponent.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Component.h"


LuaComponent::LuaComponent(lua_State * l)
{
	this->l = l;

	lua_pushstring(l, "Luacon_ci");
	lua_gettable(l, LUA_REGISTRYINDEX);
	ci = (LuaScriptInterface*)lua_touserdata(l, -1);
	lua_pop(l, 1);
}

int LuaComponent::position(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		component->Position = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		return 0;
	}
	else
	{
		lua_pushinteger(l, component->Position.X);
		lua_pushinteger(l, component->Position.Y);
		return 2;
	}
}

int LuaComponent::size(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		component->Size = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		component->Invalidate();
		return 0;
	}
	else
	{
		lua_pushinteger(l, component->Size.X);
		lua_pushinteger(l, component->Size.Y);
		return 2;
	}
}

int LuaComponent::visible(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TBOOLEAN);
		component->Visible = lua_toboolean(l, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(l, component->Visible);
		return 1;
	}
}

LuaComponent::~LuaComponent()
{
	if(component->GetParentWindow())
		component->GetParentWindow()->RemoveComponent(component);
	delete component;
}
#endif
