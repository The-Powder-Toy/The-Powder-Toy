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
	class Label;
}

class LuaScriptInterface;

class LuaLabel: public LuaComponent
{
	ui::Label * label;
	int text(lua_State * l);
public:
	static const char className[];
	static Luna<LuaLabel>::RegType methods[];

	LuaLabel(lua_State * l);
	~LuaLabel();
};