#pragma once

#include "LuaLuna.h"
#include "LuaSmartRef.h"

namespace ui
{
	class Component;
}

class LuaScriptInterface;

class LuaComponentCallback : public LuaSmartRef
{
public:
	using LuaSmartRef::LuaSmartRef;
	int CheckAndAssignArg1(lua_State *l);
};

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
	int owner_ref;

	ui::Component * GetComponent() { return component; }
	LuaComponent(lua_State * l);
	~LuaComponent();
};
