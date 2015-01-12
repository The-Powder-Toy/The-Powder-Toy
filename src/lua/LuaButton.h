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
	int actionFunction;
	void triggerAction();
	int action(lua_State * l);
	int text(lua_State * l);
	int enabled(lua_State * l);
public:
	static const char className[];
	static Luna<LuaButton>::RegType methods[];

	LuaButton(lua_State * l);
	~LuaButton();
};
