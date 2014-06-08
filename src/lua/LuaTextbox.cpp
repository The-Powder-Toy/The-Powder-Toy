#ifdef LUACONSOLE

#include <iostream>
#include "LuaScriptInterface.h"
#include "LuaTextbox.h"
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
	onTextChangedFunction(0)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	std::string text = luaL_optstring(l, 5, "");
	std::string placeholder = luaL_optstring(l, 6, "");

	textbox = new ui::Textbox(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text, placeholder);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	class TextChangedAction : public ui::TextboxAction
	{
		LuaTextbox * t;
	public:
		TextChangedAction(LuaTextbox * t) : t(t) {}
		void TextChangedCallback(ui::Textbox * sender)
		{
			t->triggerOnTextChanged();
		}
	};
	textbox->SetActionCallback(new TextChangedAction(this));
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
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onTextChangedFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onTextChangedFunction = 0;
	}
	return 0;
}

void LuaTextbox::triggerOnTextChanged()
{
	if(onTextChangedFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onTextChangedFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, UserData);
		if (lua_pcall(l, 1, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

int LuaTextbox::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		textbox->SetText(std::string(lua_tostring(l, 1)));
		return 0;
	}
	else
	{
		lua_pushstring(l, textbox->GetText().c_str());
		return 1;
	}
}

LuaTextbox::~LuaTextbox()
{
}
#endif
