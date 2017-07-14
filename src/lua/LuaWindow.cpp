#ifdef LUACONSOLE

#include <iostream>
#include "LuaScriptInterface.h"
#include "LuaWindow.h"
#include "LuaButton.h"
#include "LuaLabel.h"
#include "LuaTextbox.h"
#include "LuaCheckbox.h"
#include "LuaSlider.h"
#include "LuaProgressBar.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Window.h"

const char LuaWindow::className[] = "Window";

#define method(class, name) {#name, &class::name}
Luna<LuaWindow>::RegType LuaWindow::methods[] = {
	method(LuaWindow, position),
	method(LuaWindow, size),
	method(LuaWindow, addComponent),
	method(LuaWindow, removeComponent),
	method(LuaWindow, onInitialized),
	method(LuaWindow, onExit),
	method(LuaWindow, onTick),
	method(LuaWindow, onDraw),
	method(LuaWindow, onFocus),
	method(LuaWindow, onBlur),
	method(LuaWindow, onTryExit),
	method(LuaWindow, onTryOkay),
	method(LuaWindow, onMouseMove),
	method(LuaWindow, onMouseDown),
	method(LuaWindow, onMouseUp),
	method(LuaWindow, onMouseWheel),
	method(LuaWindow, onKeyPress),
	method(LuaWindow, onKeyRelease),
	{0, 0}
};

LuaWindow::LuaWindow(lua_State * l) :
	onInitializedFunction(0),
	onExitFunction(0),
	onTickFunction(0),
	onDrawFunction(0),
	onFocusFunction(0),
	onBlurFunction(0),
	onTryExitFunction(0),
	onTryOkayFunction(0),
	onMouseMoveFunction(0),
	onMouseDownFunction(0),
	onMouseUpFunction(0),
	onMouseWheelFunction(0),
	onKeyPressFunction(0),
	onKeyReleaseFunction(0)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 1);
	int posY = luaL_optinteger(l, 2, 1);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);

	// We should replace this with errors
	if (posX < 1 && posX != -1)
		posX = 1;
	if (posY < 1 && posY != -1)
		posY = 1;
	if (sizeX < 10)
		sizeX = 10;
	if (sizeY < 10)
		sizeY = 10;

	lua_pushstring(l, "Luacon_ci");
	lua_gettable(l, LUA_REGISTRYINDEX);
	ci = (LuaScriptInterface*)lua_touserdata(l, -1);
	lua_pop(l, 1);

	class DrawnWindow : public ui::Window
	{
		LuaWindow * luaWindow;
	public:
		DrawnWindow(ui::Point position, ui::Point size, LuaWindow * luaWindow) : ui::Window(position, size), luaWindow(luaWindow) {}
		virtual void OnDraw()
		{
			Graphics * g = ui::Engine::Ref().g;
			g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
			luaWindow->triggerOnDraw();
		}
		virtual void OnInitialized() { luaWindow->triggerOnInitialized(); }
		virtual void OnExit() { luaWindow->triggerOnExit(); }
		virtual void OnTick(float dt) { luaWindow->triggerOnTick( dt); }
		virtual void OnFocus() { luaWindow->triggerOnFocus(); }
		virtual void OnBlur() { luaWindow->triggerOnBlur(); }
		virtual void OnTryExit(ExitMethod) { luaWindow->triggerOnTryExit(); }
		virtual void OnTryOkay(OkayMethod) { luaWindow->triggerOnTryOkay(); }
		virtual void OnMouseMove(int x, int y, int dx, int dy) { luaWindow->triggerOnMouseMove(x, y, dx, dy); }
		virtual void OnMouseDown(int x, int y, unsigned button) { luaWindow->triggerOnMouseDown(x, y, button); }
		virtual void OnMouseUp(int x, int y, unsigned button) { luaWindow->triggerOnMouseUp(x, y, button); }
		virtual void OnMouseWheel(int x, int y, int d) { luaWindow->triggerOnMouseWheel(x, y, d); }
		virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) { luaWindow->triggerOnKeyPress(key, character, shift, ctrl, alt); }
		virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) { luaWindow->triggerOnKeyRelease(key, character, shift, ctrl, alt); }
	};

	window = new DrawnWindow(ui::Point(posX, posY), ui::Point(sizeX, sizeY), this);
}

int LuaWindow::addComponent(lua_State * l)
{
	void * luaComponent = NULL;
	ui::Component * component = NULL;
	if ((luaComponent = Luna<LuaButton>::tryGet(l, 1)))
		component = Luna<LuaButton>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaLabel>::tryGet(l, 1)))
		component = Luna<LuaLabel>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaTextbox>::tryGet(l, 1)))
		component = Luna<LuaTextbox>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaCheckbox>::tryGet(l, 1)))
		component = Luna<LuaCheckbox>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaSlider>::tryGet(l, 1)))
		component = Luna<LuaSlider>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaProgressBar>::tryGet(l, 1)))
		component = Luna<LuaProgressBar>::get(luaComponent)->GetComponent();
	else
		luaL_typerror(l, 1, "Component");
	if(component)
		window->AddComponent(component);
	return 0;
}

int LuaWindow::removeComponent(lua_State * l)
{
	void * luaComponent = NULL;
	ui::Component * component = NULL;
	if ((luaComponent = Luna<LuaButton>::tryGet(l, 1)))
		component = Luna<LuaButton>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaLabel>::tryGet(l, 1)))
		component = Luna<LuaLabel>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaTextbox>::tryGet(l, 1)))
		component = Luna<LuaTextbox>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaCheckbox>::tryGet(l, 1)))
		component = Luna<LuaCheckbox>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaSlider>::tryGet(l, 1)))
		component = Luna<LuaSlider>::get(luaComponent)->GetComponent();
	else if ((luaComponent = Luna<LuaProgressBar>::tryGet(l, 1)))
		component = Luna<LuaProgressBar>::get(luaComponent)->GetComponent();
	else
		luaL_typerror(l, 1, "Component");
	if(component)
		window->RemoveComponent(component);
	return 0;
}

int LuaWindow::position(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		int posX = lua_tointeger(l, 1);
		int posY = lua_tointeger(l, 2);

		if (posX < 1 || posY < 1)
		{
			return luaL_error(l, "Invalid position: '%d,%d'", posX, posY);
		}
		window->Position = ui::Point(posX, posY);
		return 0;
	}
	else
	{
		lua_pushinteger(l, window->Position.X);
		lua_pushinteger(l, window->Position.Y);
		return 2;
	}
}

int LuaWindow::size(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		int sizeX = lua_tointeger(l, 1);
		int sizeY = lua_tointeger(l, 2);

		if (sizeX < 10 || sizeY < 10)
		{
			return luaL_error(l, "Invalid size: '%d,%d'", sizeX, sizeY);
		}
		window->Size = ui::Point(sizeX, sizeY);
		return 0;
	}
	else
	{
		lua_pushinteger(l, window->Size.X);
		lua_pushinteger(l, window->Size.Y);
		return 2;
	}
}

void LuaWindow::triggerOnInitialized()
{
	if(onInitializedFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onInitializedFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnExit()
{
	if(onExitFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onExitFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnTick(float dt)
{
	if(onTickFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onTickFunction);
		lua_pushnumber(l, dt);
		if(lua_pcall(l, 1, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnDraw()
{
	if(onDrawFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onDrawFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnFocus()
{
	if(onFocusFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onFocusFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnBlur()
{
	if(onBlurFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onBlurFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnTryExit()
{
	if(onTryExitFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onTryExitFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnTryOkay()
{
	if(onTryOkayFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onTryOkayFunction);
		if(lua_pcall(l, 0, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnMouseMove(int x, int y, int dx, int dy)
{
	if(onMouseMoveFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onMouseMoveFunction);
		lua_pushinteger(l, x);
		lua_pushinteger(l, y);
		lua_pushinteger(l, dx);
		lua_pushinteger(l, dy);
		if(lua_pcall(l, 4, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnMouseDown(int x, int y, unsigned button)
{
	if(onMouseDownFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onMouseDownFunction);
		lua_pushinteger(l, x);
		lua_pushinteger(l, y);
		lua_pushinteger(l, button);
		if(lua_pcall(l, 3, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnMouseUp(int x, int y, unsigned button)
{
	if(onMouseUpFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onMouseUpFunction);
		lua_pushinteger(l, x);
		lua_pushinteger(l, y);
		lua_pushinteger(l, button);
		if(lua_pcall(l, 3, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnMouseWheel(int x, int y, int d)
{
	if(onMouseWheelFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onMouseWheelFunction);
		lua_pushinteger(l, x);
		lua_pushinteger(l, y);
		lua_pushinteger(l, d);
		if(lua_pcall(l, 3, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(onKeyPressFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onKeyPressFunction);
		lua_pushinteger(l, key);
		lua_pushinteger(l, character);
		lua_pushboolean(l, shift);
		lua_pushboolean(l, ctrl);
		lua_pushboolean(l, alt);
		if(lua_pcall(l, 5, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

void LuaWindow::triggerOnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(onKeyReleaseFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onKeyReleaseFunction);
		lua_pushinteger(l, key);
		lua_pushinteger(l, character);
		lua_pushboolean(l, shift);
		lua_pushboolean(l, ctrl);
		lua_pushboolean(l, alt);
		if(lua_pcall(l, 5, 0, 0))
		{
			ci->Log(CommandInterface::LogError, lua_tostring(l, -1));
		}
	}
}

int LuaWindow::onInitialized(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onInitializedFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onInitializedFunction = 0;
	}
	return 0;
}

int LuaWindow::onExit(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onExitFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onExitFunction = 0;
	}
	return 0;
}

int LuaWindow::onTick(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onTickFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onTickFunction = 0;
	}
	return 0;
}

int LuaWindow::onDraw(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onDrawFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onDrawFunction = 0;
	}
	return 0;
}

int LuaWindow::onFocus(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onFocusFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onFocusFunction = 0;
	}
	return 0;
}

int LuaWindow::onBlur(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onBlurFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onBlurFunction = 0;
	}
	return 0;
}

int LuaWindow::onTryExit(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onTryExitFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onTryExitFunction = 0;
	}
	return 0;
}

int LuaWindow::onTryOkay(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onTryOkayFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onTryOkayFunction = 0;
	}
	return 0;
}

int LuaWindow::onMouseMove(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onMouseMoveFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onMouseMoveFunction = 0;
	}
	return 0;
}

int LuaWindow::onMouseDown(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onMouseDownFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onMouseDownFunction = 0;
	}
	return 0;
}

int LuaWindow::onMouseUp(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onMouseUpFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onMouseUpFunction = 0;
	}
	return 0;
}

int LuaWindow::onMouseWheel(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onMouseWheelFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onMouseWheelFunction = 0;
	}
	return 0;
}

int LuaWindow::onKeyPress(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onKeyPressFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onKeyPressFunction = 0;
	}
	return 0;
}

int LuaWindow::onKeyRelease(lua_State * l)
{
	if(lua_type(l, 1) != LUA_TNIL)
	{
		luaL_checktype(l, 1, LUA_TFUNCTION);
		lua_pushvalue(l, 1);
		onKeyReleaseFunction = luaL_ref(l, LUA_REGISTRYINDEX);
	}
	else
	{
		onKeyReleaseFunction = 0;
	}
	return 0;
}


LuaWindow::~LuaWindow()
{
	window->CloseActiveWindow();
	delete window;
}
#endif
