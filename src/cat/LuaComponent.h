#pragma once

extern "C" {
	#include "lua5.1/lua.h"
	#include "lua5.1/lauxlib.h"
	#include "lua5.1/lualib.h"
}

#include "LuaLuna.h"

namespace ui
{
	class Component;
}

class LuaScriptInterface;

class LuaComponent
{
protected:
	ui::Component * component;
	lua_State * l;
	int position(lua_State * l);
	int size(lua_State * l);
	int visible(lua_State * l);
public:
	LuaScriptInterface * ci;
	int UserData;

	ui::Component * GetComponent() { return component; }
	LuaComponent(lua_State * l);
	~LuaComponent();
};