#pragma once

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class Checkbox;
}

class LuaScriptInterface;

class LuaCheckbox: public LuaComponent
{
	ui::Checkbox * checkbox;
	LuaComponentCallback actionFunction;
	void triggerAction();
	int action(lua_State *L);
	int checked(lua_State *L);
	int text(lua_State *L);
public:
	static const char className[];
	static Luna<LuaCheckbox>::RegType methods[];

	LuaCheckbox(lua_State *L);
	~LuaCheckbox();
};
