#include "Config.h"
#ifdef LUACONSOLE

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
	actionFunction(l)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	String text = tpt_lua_optString(l, 5, "");
	String toolTip = tpt_lua_optString(l, 6, "");

	button = new ui::Button(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, toolTip);
	component = button;
	button->SetActionCallback({ [this] { triggerAction(); } });
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
	return actionFunction.CheckAndAssignArg1(l);
}

int LuaButton::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		button->SetText(tpt_lua_checkString(l, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(l, button->GetText());
		return 1;
	}
}

void LuaButton::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, owner_ref);
		if (lua_pcall(l, 1, 0, 0))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(l, -1));
		}
	}
}

LuaButton::~LuaButton()
{
}
#endif
