#pragma once

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "LuaLuna.h"

namespace ui
{
	class Label;
}

class LuaLabel
{
	ui::Label * label;
	lua_State * l;
	int text(lua_State * l);
	int position(lua_State * l);
	int size(lua_State * l);
public:
	static const char className[];
	static Luna<LuaLabel>::RegType methods[];

	ui::Label * GetComponent() { return label; }
	LuaLabel(lua_State * l);
	~LuaLabel();
};