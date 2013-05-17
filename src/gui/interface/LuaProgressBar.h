#pragma once

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "LuaLuna.h"
#include "LuaComponent.h"

namespace ui
{
	class ProgressBar;
}

class LuaScriptInterface;

class LuaProgressBar: public LuaComponent
{
	ui::ProgressBar * progressBar;
	int onValueChangedFunction;
	int value(lua_State * l);
public:
	static const char className[];
	static Luna<LuaProgressBar>::RegType methods[];

	LuaProgressBar(lua_State * l);
	~LuaProgressBar();
};
