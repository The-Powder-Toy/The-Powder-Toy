#include "LuaSlider.h"
#include "LuaScriptInterface.h"
#include "gui/interface/Slider.h"

const char LuaSlider::className[] = "slider";

#define method(class, name) {#name, &class::name}
Luna<LuaSlider>::RegType LuaSlider::methods[] = {
	method(LuaSlider, onValueChanged),
	method(LuaSlider, position),
	method(LuaSlider, size),
	method(LuaSlider, visible),
	method(LuaSlider, value),
	method(LuaSlider, steps),
	{nullptr, nullptr}
};

LuaSlider::LuaSlider(lua_State *L) :
	LuaComponent(L)
{
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	int steps = luaL_optinteger(L, 5, 10);

	slider = new ui::Slider(ui::Point(posX, posY), ui::Point(sizeX, sizeY), steps);
	component = slider;
	slider->SetActionCallback({ [this] { triggerOnValueChanged(); } });
}

int LuaSlider::steps(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		slider->SetSteps(lua_tointeger(L, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(L, slider->GetSteps());
		return 1;
	}
}

int LuaSlider::onValueChanged(lua_State *L)
{
	return onValueChangedFunction.CheckAndAssignArg1(L);
}

int LuaSlider::value(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		slider->SetValue(lua_tointeger(L, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(L, slider->GetValue());
		return 1;
	}
}

void LuaSlider::triggerOnValueChanged()
{
	if(onValueChangedFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onValueChangedFunction);
		lua_rawgeti(L, LUA_REGISTRYINDEX, owner_ref);
		lua_pushinteger(L, slider->GetValue());
		if (tpt_lua_pcall(L, 2, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

LuaSlider::~LuaSlider()
{
}
