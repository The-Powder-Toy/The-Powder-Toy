#pragma once

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "LuaLuna.h"

namespace ui
{
	class Button;
}

class LuaButton
{
	ui::Button * button;
	int actionFunction;
	lua_State * l;
	void triggerAction();
	int action(lua_State * l);
	int text(lua_State * l);
	int position(lua_State * l);
	int size(lua_State * l);
public:
	static const char className[];
	static Luna<LuaButton>::RegType methods[];

	ui::Button * GetComponent() { return button; }
	LuaButton(lua_State * l);
	~LuaButton();
};