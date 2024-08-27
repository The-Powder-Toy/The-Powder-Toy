#include "LuaCheckbox.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Checkbox.h"

const char LuaCheckbox::className[] = "checkbox";

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

LuaCheckbox::LuaCheckbox(lua_State *L) :
	LuaComponent(L)
{
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	String text = tpt_lua_optString(L, 5, "");

	checkbox = new ui::Checkbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, "");
	component = checkbox;
	checkbox->SetActionCallback({ [this] { triggerAction(); } });
}

int LuaCheckbox::checked(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		checkbox->SetChecked(lua_toboolean(L, 1));
		return 0;
	}
	else
	{
		lua_pushboolean(L, checkbox->GetChecked());
		return 1;
	}
}

int LuaCheckbox::action(lua_State *L)
{
	return actionFunction.CheckAndAssignArg1(L);
}

int LuaCheckbox::text(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		checkbox->SetText(tpt_lua_checkString(L, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(L, checkbox->GetText());
		return 1;
	}
}

void LuaCheckbox::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, owner_ref);
		lua_pushboolean(L, checkbox->GetChecked());
		if (tpt_lua_pcall(L, 2, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

LuaCheckbox::~LuaCheckbox()
{
}
