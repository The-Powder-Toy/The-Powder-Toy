#ifdef LUACONSOLE

#include <iostream>
#include "LuaCheckbox.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Checkbox.h"

const char LuaCheckbox::className[] = "Checkbox";

#define method(class, name) {#name, &class::name}
Luna<LuaCheckbox>::RegType LuaCheckbox::methods[] = {
	method(LuaCheckbox, action),
	method(LuaCheckbox, text),
	method(LuaCheckbox, position),
	method(LuaCheckbox, size),
	method(LuaCheckbox, visible),
	method(LuaCheckbox, checked),
	{0, 0}
};

LuaCheckbox::LuaCheckbox(lua_State * l) :
	LuaComponent(l),
	actionFunction(0)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	std::string text = luaL_optstring(l, 5, "");

	checkbox = new ui::Checkbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, "");
	component = checkbox;
	class ClickAction : public ui::CheckboxAction
	{
		LuaCheckbox * luaCheckbox;
	public:
		ClickAction(LuaCheckbox * luaCheckbox) : luaCheckbox(luaCheckbox) {}
		void ActionCallback(ui::Checkbox * sender)
		{
			luaCheckbox->triggerAction();
		}
	};
	checkbox->SetActionCallback(new ClickAction(this));
}

int LuaCheckbox::checked(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		checkbox->SetChecked(lua_toboolean(l, 1));
		return 0;
	}
	else
	{
		lua_pushboolean(l, checkbox->GetChecked());
		return 1;
	}
}

int LuaCheckbox::action(lua_State * l)
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

int LuaCheckbox::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		checkbox->SetText(lua_tostring(l, 1));
		return 0;
	}
	else
	{
		lua_pushstring(l, checkbox->GetText().c_str());
		return 1;
	}
}

void LuaCheckbox::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, UserData);
		lua_pushboolean(l, checkbox->GetChecked());
		if (lua_pcall(l, 2, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

LuaCheckbox::~LuaCheckbox()
{
}
#endif
