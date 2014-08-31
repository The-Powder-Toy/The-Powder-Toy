#ifdef LUACONSOLE

#include <iostream>
#include "LuaButton.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Button.h"

const char LuaButton::className[] = "Button";

#define method(class, name) {#name, &class::name}
Luna<LuaButton>::RegType LuaButton::methods[] = {
	method(LuaButton, action),
	method(LuaButton, text),
	method(LuaButton, position),
	method(LuaButton, size),
	method(LuaButton, visible),
	method(LuaButton, enabled),
	{0, 0}
};

LuaButton::LuaButton(lua_State * l) :
	LuaComponent(l),
	actionFunction(0)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	std::string text = luaL_optstring(l, 5, "");
	std::string toolTip = luaL_optstring(l, 6, "");

	button = new ui::Button(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, toolTip);
	component = button;
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

int LuaButton::enabled(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TBOOLEAN);
		button->Enabled = lua_toboolean(l, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(l, button->Enabled);
		return 1;
	}
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
	return 0;
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

void LuaButton::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, UserData);
		if (lua_pcall(l, 1, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

LuaButton::~LuaButton()
{
}
#endif
