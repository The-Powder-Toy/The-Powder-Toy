#ifdef LUACONSOLE

#include "LuaWindow.h"

#include "LuaScriptInterface.h"
#include "LuaButton.h"
#include "LuaLabel.h"
#include "LuaTextbox.h"
#include "LuaCheckbox.h"
#include "LuaSlider.h"
#include "LuaProgressBar.h"

#include "gui/interface/Window.h"
#include "gui/interface/Engine.h"

#include "graphics/Graphics.h"

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
	onInitializedFunction(l),
	onExitFunction(l),
	onTickFunction(l),
	onDrawFunction(l),
	onFocusFunction(l),
	onBlurFunction(l),
	onTryExitFunction(l),
	onTryOkayFunction(l),
	onMouseMoveFunction(l),
	onMouseDownFunction(l),
	onMouseUpFunction(l),
	onMouseWheelFunction(l),
	onKeyPressFunction(l),
	onKeyReleaseFunction(l)
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
		void OnDraw() override
		{
			Graphics * g = ui::Engine::Ref().g;
			g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
			luaWindow->triggerOnDraw();
		}
		void OnInitialized() override { luaWindow->triggerOnInitialized(); }
		void OnExit() override { luaWindow->triggerOnExit(); }
		void OnTick(float dt) override { luaWindow->triggerOnTick( dt); }
		void OnFocus() override { luaWindow->triggerOnFocus(); }
		void OnBlur() override { luaWindow->triggerOnBlur(); }
		void OnTryExit(ExitMethod) override { luaWindow->triggerOnTryExit(); }
		void OnTryOkay(OkayMethod) override { luaWindow->triggerOnTryOkay(); }
		void OnMouseMove(int x, int y, int dx, int dy) override { luaWindow->triggerOnMouseMove(x, y, dx, dy); }
		void OnMouseDown(int x, int y, unsigned button) override { luaWindow->triggerOnMouseDown(x, y, button); }
		void OnMouseUp(int x, int y, unsigned button) override { luaWindow->triggerOnMouseUp(x, y, button); }
		void OnMouseWheel(int x, int y, int d) override { luaWindow->triggerOnMouseWheel(x, y, d); }
		void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override { luaWindow->triggerOnKeyPress(key, scan, repeat, shift, ctrl, alt); }
		void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override { luaWindow->triggerOnKeyRelease(key, scan, repeat, shift, ctrl, alt); }
	};

	window = new DrawnWindow(ui::Point(posX, posY), ui::Point(sizeX, sizeY), this);
}

int LuaWindow::addComponent(lua_State * l)
{
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(l, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(l, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(l, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(l, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(l, 1, "Component");
	if (luaComponent)
	{
		auto ok = grabbed_components.insert(std::make_pair(luaComponent, LuaSmartRef(l)));
		if (ok.second)
		{
			auto it = ok.first;
			it->second.Assign(l, 1);
			it->first->owner_ref = it->second;
		}
		window->AddComponent(luaComponent->GetComponent());
	}
	return 0;
}

int LuaWindow::removeComponent(lua_State * l)
{
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(l, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(l, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(l, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(l, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(l, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(l, 1, "Component");
	if (luaComponent)
	{
		ui::Component *component = luaComponent->GetComponent();
		window->RemoveComponent(component);
		auto it = grabbed_components.find(luaComponent);
		if (it != grabbed_components.end())
		{
			it->second.Clear();
			it->first->owner_ref = it->second;
			grabbed_components.erase(it);
		}
	}
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
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
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
		}
	}
}

void LuaWindow::triggerOnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if(onKeyPressFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onKeyPressFunction);
		lua_pushinteger(l, key);
		lua_pushinteger(l, scan);
		lua_pushboolean(l, shift);
		lua_pushboolean(l, ctrl);
		lua_pushboolean(l, alt);
		if(lua_pcall(l, 5, 0, 0))
		{
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
		}
	}
}

void LuaWindow::triggerOnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if(onKeyReleaseFunction)
	{
		lua_rawgeti(l, LUA_REGISTRYINDEX, onKeyReleaseFunction);
		lua_pushinteger(l, key);
		lua_pushinteger(l, scan);
		lua_pushboolean(l, shift);
		lua_pushboolean(l, ctrl);
		lua_pushboolean(l, alt);
		if(lua_pcall(l, 5, 0, 0))
		{
			ci->Log(CommandInterface::LogError, ByteString(lua_tostring(l, -1)).FromUtf8());
		}
	}
}

int LuaWindow::onInitialized(lua_State * l)
{
	return onInitializedFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onExit(lua_State * l)
{
	return onExitFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onTick(lua_State * l)
{
	return onTickFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onDraw(lua_State * l)
{
	return onDrawFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onFocus(lua_State * l)
{
	return onFocusFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onBlur(lua_State * l)
{
	return onBlurFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onTryExit(lua_State * l)
{
	return onTryExitFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onTryOkay(lua_State * l)
{
	return onTryOkayFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onMouseMove(lua_State * l)
{
	return onMouseMoveFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onMouseDown(lua_State * l)
{
	return onMouseDownFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onMouseUp(lua_State * l)
{
	return onMouseUpFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onMouseWheel(lua_State * l)
{
	return onMouseWheelFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onKeyPress(lua_State * l)
{
	return onKeyPressFunction.CheckAndAssignArg1(l);
}

int LuaWindow::onKeyRelease(lua_State * l)
{
	return onKeyReleaseFunction.CheckAndAssignArg1(l);
}


LuaWindow::~LuaWindow()
{
	for (auto &component_and_ref : grabbed_components)
	{
		window->RemoveComponent(component_and_ref.first->GetComponent());
		component_and_ref.second.Clear();
		component_and_ref.first->owner_ref = component_and_ref.second;
	}
	window->CloseActiveWindow();
	delete window;
}
#endif
