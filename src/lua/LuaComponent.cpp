#include "LuaComponent.h"
#include "LuaScriptInterface.h"
#include "LuaWindow.h"
#include "gui/interface/Component.h"
#include "gui/interface/Window.h"

int LuaComponentCallback::CheckAndAssignArg1(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TNIL)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);
	}
	LuaSmartRef::Assign(L, 1);
	return 0;
}

LuaComponent::LuaComponent(lua_State *L) : component(nullptr), owner_ref(LUA_REFNIL)
{
	this->L = L; // I don't get how this doesn't cause crashes later on
	
	ci = static_cast<LuaScriptInterface *>(&CommandInterface::Ref());
}

int LuaComponent::position(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		luaL_checktype(L, 2, LUA_TNUMBER);
		component->Position = ui::Point(lua_tointeger(L, 1), lua_tointeger(L, 2));
		return 0;
	}
	else
	{
		lua_pushinteger(L, component->Position.X);
		lua_pushinteger(L, component->Position.Y);
		return 2;
	}
}

int LuaComponent::size(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		luaL_checktype(L, 2, LUA_TNUMBER);
		component->Size = ui::Point(lua_tointeger(L, 1), lua_tointeger(L, 2));
		component->Invalidate();
		return 0;
	}
	else
	{
		lua_pushinteger(L, component->Size.X);
		lua_pushinteger(L, component->Size.Y);
		return 2;
	}
}

int LuaComponent::visible(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		component->Visible = lua_toboolean(L, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(L, component->Visible);
		return 1;
	}
}

LuaComponent::~LuaComponent()
{
	if (parent)
		parent->ClearRef(this);

	if (component)
	{
		if (component->GetParentWindow())
			component->GetParentWindow()->RemoveComponent(component);
		delete component;
	}
}
