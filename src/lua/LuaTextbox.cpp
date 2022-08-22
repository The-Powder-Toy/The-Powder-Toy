#include "Config.h"
#ifdef LUACONSOLE

#include "LuaTextbox.h"

#include "LuaScriptInterface.h"

#include "gui/interface/Textbox.h"

const char LuaTextbox::className[] = "Textbox";

#define method(class, name) {#name, &class::name}
Luna<LuaTextbox>::RegType LuaTextbox::methods[] = {
	method(LuaTextbox, text),
	method(LuaTextbox, readonly),
	method(LuaTextbox, onTextChanged),
	method(LuaTextbox, position),
	method(LuaTextbox, size),
	method(LuaTextbox, visible),
	{0, 0}
};

LuaTextbox::LuaTextbox(lua_State * l) :
	LuaComponent(l),
	onTextChangedFunction(l)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	String text = tpt_lua_optString(l, 5, "");
	String placeholder = tpt_lua_optString(l, 6, "");

	textbox = new ui::Textbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, placeholder);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textbox->SetActionCallback({ [this] { triggerOnTextChanged(); } });
	component = textbox;
}

int LuaTextbox::readonly(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TBOOLEAN);
		textbox->ReadOnly = lua_toboolean(l, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(l, textbox->ReadOnly);
		return 1;
	}
}

int LuaTextbox::onTextChanged(lua_State * l)
{
	return onTextChangedFunction.CheckAndAssignArg1(l);
}

void LuaTextbox::triggerOnTextChanged()
{
	if(onTextChangedFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onTextChangedFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, owner_ref);
		if (lua_pcall(l, 1, 0, 0))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_optString(l, -1));
		}
	}
}

int LuaTextbox::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		textbox->SetText(tpt_lua_checkString(l, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(l, textbox->GetText());
		return 1;
	}
}

LuaTextbox::~LuaTextbox()
{
}
#endif
