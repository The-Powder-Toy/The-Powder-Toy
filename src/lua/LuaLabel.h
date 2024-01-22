#pragma once

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
	int text(lua_State *L);
public:
	static const char className[];
	static Luna<LuaLabel>::RegType methods[];

	LuaLabel(lua_State *L);
	~LuaLabel();
};
