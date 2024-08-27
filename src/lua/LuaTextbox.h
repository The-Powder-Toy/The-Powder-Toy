#pragma once

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class Textbox;
}

class LuaScriptInterface;

class LuaTextbox: public LuaComponent
{
	LuaComponentCallback onTextChangedFunction;
	ui::Textbox * textbox;
	int text(lua_State *L);
	int readonly(lua_State *L);
	int onTextChanged(lua_State *L);
	void triggerOnTextChanged();
public:
	static const char className[];
	static Luna<LuaTextbox>::RegType methods[];

	LuaTextbox(lua_State *L);
	~LuaTextbox();
};
