#pragma once

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
