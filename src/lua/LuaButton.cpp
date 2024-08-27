#include "LuaButton.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Button.h"

const char LuaButton::className[] = "button";

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

LuaButton::LuaButton(lua_State *L) :
	LuaComponent(L)
{
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	String text = tpt_lua_optString(L, 5, "");
	String toolTip = tpt_lua_optString(L, 6, "");

	button = new ui::Button(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, toolTip);
	component = button;
	button->SetActionCallback({ [this] { triggerAction(); } });
}

int LuaButton::enabled(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		button->Enabled = lua_toboolean(L, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(L, button->Enabled);
		return 1;
	}
}

int LuaButton::action(lua_State *L)
{
	return actionFunction.CheckAndAssignArg1(L);
}

int LuaButton::text(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		button->SetText(tpt_lua_checkString(L, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(L, button->GetText());
		return 1;
	}
}

void LuaButton::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, owner_ref);
		if (tpt_lua_pcall(L, 1, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

LuaButton::~LuaButton()
{
}
