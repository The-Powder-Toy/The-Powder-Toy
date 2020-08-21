#ifdef LUACONSOLE

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
	actionFunction(l)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	String text = ByteString(luaL_optstring(l, 5, "")).FromUtf8();

	checkbox = new ui::Checkbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, "");
	component = checkbox;
	checkbox->SetActionCallback({ [this] { triggerAction(); } });
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
	return actionFunction.CheckAndAssignArg1(l);
}

int LuaCheckbox::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		checkbox->SetText(ByteString(lua_tostring(l, 1)).FromUtf8());
		return 0;
	}
	else
	{
		lua_pushstring(l, checkbox->GetText().ToUtf8().c_str());
		return 1;
	}
}

void LuaCheckbox::triggerAction()
{
	if(actionFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, actionFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, owner_ref);
		lua_pushboolean(l, checkbox->GetChecked());
		if (lua_pcall(l, 2, 0, 0))
		{
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
		}
	}
}

LuaCheckbox::~LuaCheckbox()
{
}
#endif
