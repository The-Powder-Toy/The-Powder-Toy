extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <iostream>
#include "LuaButton.h"
#include "interface/Button.h"

const char LuaButton::className[] = "Button";

#define method(class, name) {#name, &class::name}
Luna<LuaButton>::RegType LuaButton::methods[] = {
	method(LuaButton, action),
	method(LuaButton, text),
	method(LuaButton, position),
	method(LuaButton, size),
	{0, 0}
};

LuaButton::LuaButton(lua_State * l) :
	actionFunction(0)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	std::string text = luaL_optstring(l, 5, "");
	std::string toolTip = luaL_optstring(l, 6, "");;

	button = new ui::Button(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, toolTip);
	class ClickAction : public ui::ButtonAction
	{
		LuaButton * luaButton;
	public:
		ClickAction(LuaButton * luaButton) : luaButton(luaButton) {}
		void ActionCallback(ui::Button * sender)
		{
			luaButton->triggerAction();
		}
	};
	button->SetActionCallback(new ClickAction(this));
}

int LuaButton::action(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		actionFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		actionFunction = 0;
	}
}

int LuaButton::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TSTRING);
		button->SetText(lua_tostring(l, 1));
		return 0;
	}
	else
	{
		lua_pushstring(l, button->GetText().c_str());
		return 1;
	}
}

int LuaButton::position(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		button->Position = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		return 0;
	}
	else
	{
		lua_pushinteger(l, button->Position.X);
		lua_pushinteger(l, button->Position.Y);
		return 2;
	}
}

int LuaButton::size(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		button->Size = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		button->Invalidate();
		return 0;
	}
	else
	{
		lua_pushinteger(l, button->Size.X);
		lua_pushinteger(l, button->Size.Y);
		return 2;
	}
}

void LuaButton::triggerAction()
{
	if(actionFunction)
	{
		std::cout << actionFunction << std::endl;
		lua_rawgeti(l, LUA_REGISTRYINDEX, actionFunction);
		lua_pushinteger(l, 1);
		if (lua_pcall(l, 1, 0, 0))
		{
			//Log error somewhere
		}
	}
}

LuaButton::~LuaButton()
{
	if(button->GetParentWindow())
		button->GetParentWindow()->RemoveComponent(button);
	delete button;
}