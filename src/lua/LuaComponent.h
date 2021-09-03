#pragma once

#include "LuaLuna.h"
#include "LuaSmartRef.h"

namespace ui
{
	class Component;
}

class LuaScriptInterface;
class LuaWindow;

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
	LuaWindow * parent = nullptr;

	int position(lua_State * l);
	int size(lua_State * l);
	int visible(lua_State * l);
public:
	LuaScriptInterface * ci;
	int owner_ref;

	ui::Component * GetComponent() { return component; }
	void SetParentWindow(LuaWindow *parent) { this->parent = parent; }
	LuaComponent(lua_State * l);
	~LuaComponent();
};
