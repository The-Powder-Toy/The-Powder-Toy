#ifdef LUACONSOLE

#include <iostream>
#include "LuaSlider.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Slider.h"

const char LuaSlider::className[] = "Slider";

#define method(class, name) {#name, &class::name}
Luna<LuaSlider>::RegType LuaSlider::methods[] = {
	method(LuaSlider, onValueChanged),
	method(LuaSlider, position),
	method(LuaSlider, size),
	method(LuaSlider, visible),
	method(LuaSlider, value),
	method(LuaSlider, steps),
	{0, 0}
};

LuaSlider::LuaSlider(lua_State * l) :
	LuaComponent(l),
	onValueChangedFunction(0)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	int steps = luaL_optinteger(l, 5, 10);

	slider = new ui::Slider(ui::Point(posX, posY), ui::Point(sizeX, sizeY), steps);
	component = slider;
	class ValueAction : public ui::SliderAction
	{
		LuaSlider * luaSlider;
	public:
		ValueAction(LuaSlider * luaSlider) : luaSlider(luaSlider) {}
		void ValueChangedCallback(ui::Slider * sender)
		{
			luaSlider->triggerOnValueChanged();
		}
	};
	slider->SetActionCallback(new ValueAction(this));
}

int LuaSlider::steps(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		slider->SetSteps(lua_tointeger(l, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(l, slider->GetSteps());
		return 1;
	}
}

int LuaSlider::onValueChanged(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onValueChangedFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onValueChangedFunction = 0;
	}
	return 0;
}

int LuaSlider::value(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		slider->SetValue(lua_tointeger(l, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(l, slider->GetValue());
		return 1;
	}
}

void LuaSlider::triggerOnValueChanged()
{
	if(onValueChangedFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onValueChangedFunction);
		lua_rawgeti(l, LUA_REGISTRYINDEX, UserData);
		lua_pushinteger(l, slider->GetValue());
		if (lua_pcall(l, 2, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

LuaSlider::~LuaSlider()
{
}
#endif
