#pragma once

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
	int progress(lua_State *L);
	int status(lua_State *L);
public:
	static const char className[];
	static Luna<LuaProgressBar>::RegType methods[];

	LuaProgressBar(lua_State *L);
	~LuaProgressBar();
};
