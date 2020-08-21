#ifdef LUACONSOLE

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
	onValueChangedFunction(l)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	int steps = luaL_optinteger(l, 5, 10);

	slider = new ui::Slider(ui::Point(posX, posY), ui::Point(sizeX, sizeY), steps);
	component = slider;
	slider->SetActionCallback({ [this] { triggerOnValueChanged(); } });
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
	return onValueChangedFunction.CheckAndAssignArg1(l);
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
		lua_rawgeti(l, LUA_REGISTRYINDEX, owner_ref);
		lua_pushinteger(l, slider->GetValue());
		if (lua_pcall(l, 2, 0, 0))
		{
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
		}
	}
}

LuaSlider::~LuaSlider()
{
}
#endif
