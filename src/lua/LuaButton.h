#pragma once

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class Button;
}

class LuaScriptInterface;

class LuaButton: public LuaComponent
{
	ui::Button * button;
	LuaComponentCallback actionFunction;
	void triggerAction();
	int action(lua_State *L);
	int text(lua_State *L);
	int enabled(lua_State *L);
public:
	static const char className[];
	static Luna<LuaButton>::RegType methods[];

	LuaButton(lua_State *L);
	~LuaButton();
};
