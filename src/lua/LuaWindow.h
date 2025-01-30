#pragma once
#include "LuaLuna.h"
#include "LuaComponent.h"
#include "LuaSmartRef.h"
#include <map>

namespace ui
{
	class Window;
}

class LuaScriptInterface;
class LuaWindow
{
	LuaComponentCallback onInitializedFunction;
	LuaComponentCallback onExitFunction;
	LuaComponentCallback onTickFunction;
	LuaComponentCallback onDrawFunction;
	LuaComponentCallback onFocusFunction;
	LuaComponentCallback onBlurFunction;
	LuaComponentCallback onTryExitFunction;
	LuaComponentCallback onTryOkayFunction;
	LuaComponentCallback onMouseMoveFunction;
	LuaComponentCallback onMouseDownFunction;
	LuaComponentCallback onMouseUpFunction;
	LuaComponentCallback onMouseWheelFunction;
	LuaComponentCallback onKeyPressFunction;
	LuaComponentCallback onKeyReleaseFunction;

	std::map<LuaComponent *, LuaSmartRef> grabbedComponents;

	ui::Window * window;
	lua_State *L;
	int position(lua_State *L);
	int size(lua_State *L);
	int addComponent(lua_State *L);
	int removeComponent(lua_State *L);

	//Set event handlers
	int onInitialized(lua_State *L);
	int onExit(lua_State *L);
	int onTick(lua_State *L);
	int onDraw(lua_State *L);
	int onFocus(lua_State *L);
	int onBlur(lua_State *L);
	int onTryExit(lua_State *L);
	int onTryOkay(lua_State *L);
	int onMouseMove(lua_State *L);
	int onMouseDown(lua_State *L);
	int onMouseUp(lua_State *L);
	int onMouseWheel(lua_State *L);
	int onKeyPress(lua_State *L);
	int onKeyRelease(lua_State *L);

	void triggerOnInitialized();
	void triggerOnExit();
	void triggerOnTick();
	void triggerOnDraw();
	void triggerOnFocus();
	void triggerOnBlur();
	void triggerOnTryExit();
	void triggerOnTryOkay();
	void triggerOnMouseMove(int x, int y, int dx, int dy);
	void triggerOnMouseDown(int x, int y, unsigned button);
	void triggerOnMouseUp(int x, int y, unsigned button);
	void triggerOnMouseWheel(int x, int y, int d);
	void triggerOnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void triggerOnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);

public:
	LuaScriptInterface * ci;
	static const char className[];
	static Luna<LuaWindow>::RegType methods[];

	ui::Window * GetWindow() { return window; }
	void ClearRef(LuaComponent *luaComponent);

	LuaWindow(lua_State *L);
	~LuaWindow();
};
