#pragma once

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class Slider;
}

class LuaScriptInterface;

class LuaSlider: public LuaComponent
{
	ui::Slider * slider;
	LuaComponentCallback onValueChangedFunction;
	void triggerOnValueChanged();
	int onValueChanged(lua_State *L);
	int steps(lua_State *L);
	int value(lua_State *L);
public:
	static const char className[];
	static Luna<LuaSlider>::RegType methods[];

	LuaSlider(lua_State *L);
	~LuaSlider();
};
