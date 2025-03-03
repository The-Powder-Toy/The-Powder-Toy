#include "LuaTextbox.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Textbox.h"

const char LuaTextbox::className[] = "textbox";

#define method(class, name) {#name, &class::name}
Luna<LuaTextbox>::RegType LuaTextbox::methods[] = {
	method(LuaTextbox, text),
	method(LuaTextbox, readonly),
	method(LuaTextbox, onTextChanged),
	method(LuaTextbox, position),
	method(LuaTextbox, size),
	method(LuaTextbox, visible),
	{nullptr, nullptr}
};

LuaTextbox::LuaTextbox(lua_State *L) :
	LuaComponent(L)
{
	this->L = L;
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	String text = tpt_lua_optString(L, 5, "");
	String placeholder = tpt_lua_optString(L, 6, "");

	textbox = new ui::Textbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, placeholder);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textbox->SetActionCallback({ [this] { triggerOnTextChanged(); } });
	component = textbox;
}

int LuaTextbox::readonly(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		textbox->ReadOnly = lua_toboolean(L, 1);
		return 0;
	}
	else
	{
		lua_pushboolean(L, textbox->ReadOnly);
		return 1;
	}
}

int LuaTextbox::onTextChanged(lua_State *L)
{
	return onTextChangedFunction.CheckAndAssignArg1(L);
}

void LuaTextbox::triggerOnTextChanged()
{
	if(onTextChangedFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onTextChangedFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, owner_ref);
		if (tpt_lua_pcall(L, 1, 0, 0, eventTraitInterface))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_optString(L, -1));
		}
	}
}

int LuaTextbox::text(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		textbox->SetText(tpt_lua_checkString(L, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(L, textbox->GetText());
		return 1;
	}
}

LuaTextbox::~LuaTextbox()
{
}
