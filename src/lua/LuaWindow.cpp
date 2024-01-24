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

const char LuaWindow::className[] = "window";

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

LuaWindow::LuaWindow(lua_State *L)
{
	this->L = L;
	int posX = luaL_optinteger(L, 1, 1);
	int posY = luaL_optinteger(L, 2, 1);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);

	// We should replace this with errors
	if (posX < 1 && posX != -1)
		posX = 1;
	if (posY < 1 && posY != -1)
		posY = 1;
	if (sizeX < 10)
		sizeX = 10;
	if (sizeY < 10)
		sizeY = 10;

	ci = static_cast<LuaScriptInterface *>(&CommandInterface::Ref());

	class DrawnWindow : public ui::Window
	{
		LuaWindow * luaWindow;
	public:
		DrawnWindow(ui::Point position, ui::Point size, LuaWindow * luaWindow) : ui::Window(position, size), luaWindow(luaWindow) {}
		void OnDraw() override
		{
			Graphics * g = ui::Engine::Ref().g;
			g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
			g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
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

int LuaWindow::addComponent(lua_State *L)
{
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(L, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(L, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(L, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(L, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(L, 1, "Component");
	if (luaComponent)
	{
		auto ok = grabbedComponents.insert(std::make_pair(luaComponent, LuaSmartRef()));
		if (ok.second)
		{
			auto it = ok.first;
			it->second.Assign(L, 1);
			it->first->owner_ref = it->second;
		}
		window->AddComponent(luaComponent->GetComponent());
		luaComponent->SetParentWindow(this);
	}
	return 0;
}

int LuaWindow::removeComponent(lua_State *L)
{
	void *opaque = nullptr;
	LuaComponent *luaComponent = nullptr;
	if ((opaque = Luna<LuaButton>::tryGet(L, 1)))
		luaComponent = Luna<LuaButton>::get(opaque);
	else if ((opaque = Luna<LuaLabel>::tryGet(L, 1)))
		luaComponent = Luna<LuaLabel>::get(opaque);
	else if ((opaque = Luna<LuaTextbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaTextbox>::get(opaque);
	else if ((opaque = Luna<LuaCheckbox>::tryGet(L, 1)))
		luaComponent = Luna<LuaCheckbox>::get(opaque);
	else if ((opaque = Luna<LuaSlider>::tryGet(L, 1)))
		luaComponent = Luna<LuaSlider>::get(opaque);
	else if ((opaque = Luna<LuaProgressBar>::tryGet(L, 1)))
		luaComponent = Luna<LuaProgressBar>::get(opaque);
	else
		luaL_typerror(L, 1, "Component");
	if (luaComponent)
	{
		ui::Component *component = luaComponent->GetComponent();
		window->RemoveComponent(component);
		auto it = grabbedComponents.find(luaComponent);
		if (it != grabbedComponents.end())
		{
			it->second.Clear();
			it->first->owner_ref = it->second;
			grabbedComponents.erase(it);
			luaComponent->SetParentWindow(nullptr);
		}
	}
	return 0;
}

int LuaWindow::position(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		luaL_checktype(L, 2, LUA_TNUMBER);
		int posX = lua_tointeger(L, 1);
		int posY = lua_tointeger(L, 2);

		if (posX < 1 || posY < 1)
		{
			return luaL_error(L, "Invalid position: '%d,%d'", posX, posY);
		}
		window->Position = ui::Point(posX, posY);
		return 0;
	}
	else
	{
		lua_pushinteger(L, window->Position.X);
		lua_pushinteger(L, window->Position.Y);
		return 2;
	}
}

int LuaWindow::size(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		luaL_checktype(L, 2, LUA_TNUMBER);
		int sizeX = lua_tointeger(L, 1);
		int sizeY = lua_tointeger(L, 2);

		if (sizeX < 10 || sizeY < 10)
		{
			return luaL_error(L, "Invalid size: '%d,%d'", sizeX, sizeY);
		}
		window->Size = ui::Point(sizeX, sizeY);
		return 0;
	}
	else
	{
		lua_pushinteger(L, window->Size.X);
		lua_pushinteger(L, window->Size.Y);
		return 2;
	}
}

void LuaWindow::triggerOnInitialized()
{
	if(onInitializedFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onInitializedFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnExit()
{
	if(onExitFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onExitFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnTick(float dt)
{
	if(onTickFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onTickFunction);
		lua_pushnumber(L, dt);
		if(tpt_lua_pcall(L, 1, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnDraw()
{
	if(onDrawFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onDrawFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnFocus()
{
	if(onFocusFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onFocusFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnBlur()
{
	if(onBlurFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onBlurFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnTryExit()
{
	if(onTryExitFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onTryExitFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnTryOkay()
{
	if(onTryOkayFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onTryOkayFunction);
		if(tpt_lua_pcall(L, 0, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnMouseMove(int x, int y, int dx, int dy)
{
	if(onMouseMoveFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMouseMoveFunction);
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, dx);
		lua_pushinteger(L, dy);
		if(tpt_lua_pcall(L, 4, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnMouseDown(int x, int y, unsigned button)
{
	if(onMouseDownFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMouseDownFunction);
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, button);
		if(tpt_lua_pcall(L, 3, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnMouseUp(int x, int y, unsigned button)
{
	if(onMouseUpFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMouseUpFunction);
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, button);
		if(tpt_lua_pcall(L, 3, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnMouseWheel(int x, int y, int d)
{
	if(onMouseWheelFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onMouseWheelFunction);
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, d);
		if(tpt_lua_pcall(L, 3, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if(onKeyPressFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onKeyPressFunction);
		lua_pushinteger(L, key);
		lua_pushinteger(L, scan);
		lua_pushboolean(L, shift);
		lua_pushboolean(L, ctrl);
		lua_pushboolean(L, alt);
		if(tpt_lua_pcall(L, 5, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

void LuaWindow::triggerOnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if(onKeyReleaseFunction)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, onKeyReleaseFunction);
		lua_pushinteger(L, key);
		lua_pushinteger(L, scan);
		lua_pushboolean(L, shift);
		lua_pushboolean(L, ctrl);
		lua_pushboolean(L, alt);
		if(tpt_lua_pcall(L, 5, 0, 0, eventTraitNone))
		{
			ci->Log(CommandInterface::LogError, tpt_lua_toString(L, -1));
		}
	}
}

int LuaWindow::onInitialized(lua_State *L)
{
	return onInitializedFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onExit(lua_State *L)
{
	return onExitFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onTick(lua_State *L)
{
	return onTickFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onDraw(lua_State *L)
{
	return onDrawFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onFocus(lua_State *L)
{
	return onFocusFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onBlur(lua_State *L)
{
	return onBlurFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onTryExit(lua_State *L)
{
	return onTryExitFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onTryOkay(lua_State *L)
{
	return onTryOkayFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onMouseMove(lua_State *L)
{
	return onMouseMoveFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onMouseDown(lua_State *L)
{
	return onMouseDownFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onMouseUp(lua_State *L)
{
	return onMouseUpFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onMouseWheel(lua_State *L)
{
	return onMouseWheelFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onKeyPress(lua_State *L)
{
	return onKeyPressFunction.CheckAndAssignArg1(L);
}

int LuaWindow::onKeyRelease(lua_State *L)
{
	return onKeyReleaseFunction.CheckAndAssignArg1(L);
}

void LuaWindow::ClearRef(LuaComponent *luaComponent)
{
	auto it = grabbedComponents.find(luaComponent);
	if (it != grabbedComponents.end())
	{
		it->second.Clear();
		it->first->owner_ref = it->second;
		it->first->SetParentWindow(nullptr);
		grabbedComponents.erase(it);
	}
}

LuaWindow::~LuaWindow()
{
	for (auto &component_and_ref : grabbedComponents)
	{
		window->RemoveComponent(component_and_ref.first->GetComponent());
		component_and_ref.second.Clear();
		component_and_ref.first->owner_ref = component_and_ref.second;
		component_and_ref.first->SetParentWindow(nullptr);
	}
	window->CloseActiveWindow();
	delete window;
}
