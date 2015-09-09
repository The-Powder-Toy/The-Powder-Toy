#pragma once

#include "LuaLuna.h"

namespace ui
{
	class Window;
}

class LuaScriptInterface;
class LuaWindow
{
	int onInitializedFunction;
	int onExitFunction;
	int onTickFunction;
	int onDrawFunction;
	int onFocusFunction;
	int onBlurFunction;
	int onTryExitFunction;
	int onTryOkayFunction;
	int onMouseMoveFunction;
	int onMouseDownFunction;
	int onMouseUpFunction;
	int onMouseWheelFunction;
	int onKeyPressFunction;
	int onKeyReleaseFunction;

	ui::Window * window;
	lua_State * l;
	int position(lua_State * l);
	int size(lua_State * l);
	int addComponent(lua_State * l);
	int removeComponent(lua_State * l);

	//Set event handlers
	int onInitialized(lua_State * l);
	int onExit(lua_State * l);
	int onTick(lua_State * l);
	int onDraw(lua_State * l);
	int onFocus(lua_State * l);
	int onBlur(lua_State * l);
	int onTryExit(lua_State * l);
	int onTryOkay(lua_State * l);
	int onMouseMove(lua_State * l);
	int onMouseDown(lua_State * l);
	int onMouseUp(lua_State * l);
	int onMouseWheel(lua_State * l);
	int onKeyPress(lua_State * l);
	int onKeyRelease(lua_State * l);

	void triggerOnInitialized();
	void triggerOnExit();
	void triggerOnTick(float deltaTime);
	void triggerOnDraw();
	void triggerOnFocus();
	void triggerOnBlur();
	void triggerOnTryExit();
	void triggerOnTryOkay();
	void triggerOnMouseMove(int x, int y, int dx, int dy);
	void triggerOnMouseDown(int x, int y, unsigned button);
	void triggerOnMouseUp(int x, int y, unsigned button);
	void triggerOnMouseWheel(int x, int y, int d);
	void triggerOnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void triggerOnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

public:
	LuaScriptInterface * ci;
	int UserData;
	static const char className[];
	static Luna<LuaWindow>::RegType methods[];

	ui::Window * GetWindow() { return window; }
	LuaWindow(lua_State * l);
	~LuaWindow();
};
