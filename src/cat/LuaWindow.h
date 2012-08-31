#pragma once

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "LuaLuna.h"

namespace ui
{
	class Window;
}

class LuaWindow
{
	ui::Window * window;
	lua_State * l;
	int position(lua_State * l);
	int size(lua_State * l);
	int addComponent(lua_State * l);
public:
	static const char className[];
	static Luna<LuaWindow>::RegType methods[];

	ui::Window * GetWindow() { return window; }
	LuaWindow(lua_State * l);
	~LuaWindow();
};