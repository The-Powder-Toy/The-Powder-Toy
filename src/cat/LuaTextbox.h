#pragma once

extern "C" {
	#include "lua5.1/lua.h"
	#include "lua5.1/lauxlib.h"
	#include "lua5.1/lualib.h"
}

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class Textbox;
}

class LuaScriptInterface;

class LuaTextbox: public LuaComponent
{
	int onTextChangedFunction;
	ui::Textbox * textbox;
	int text(lua_State * l);
	int readonly(lua_State * l);
	int onTextChanged(lua_State * l);
	void triggerOnTextChanged();
public:
	static const char className[];
	static Luna<LuaTextbox>::RegType methods[];

	LuaTextbox(lua_State * l);
	~LuaTextbox();
};