extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <iostream>
#include "LuaWindow.h"
#include "LuaButton.h"
#include "interface/Button.h"
#include "interface/Window.h"

const char LuaWindow::className[] = "Window";

#define method(class, name) {#name, &class::name}
Luna<LuaWindow>::RegType LuaWindow::methods[] = {
	method(LuaWindow, position),
	method(LuaWindow, size),
	method(LuaWindow, addComponent),
	{0, 0}
};

LuaWindow::LuaWindow(lua_State * l)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);

	class DrawnWindow : public ui::Window
	{
	public:
		DrawnWindow(ui::Point position, ui::Point size) : ui::Window(position, size) {}
		virtual void OnDraw()
		{
			Graphics * g = ui::Engine::Ref().g;
			g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
		}
	};

	window = new DrawnWindow(ui::Point(posX, posY), ui::Point(sizeX, sizeY));
}

int LuaWindow::addComponent(lua_State * l)
{
	void * luaComponent = NULL;
	ui::Component * component = NULL;
	if(luaComponent = luaL_checkudata(l, 1, "Button"))
		component = Luna<LuaButton>::get(luaComponent)->GetComponent();
	else
		luaL_typerror(l, 1, "Component");
	if(component)
		window->AddComponent(component);
	return 0;
}

int LuaWindow::position(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		window->Position = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
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
		window->Size = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		return 0;
	}
	else
	{
		lua_pushinteger(l, window->Size.X);
		lua_pushinteger(l, window->Size.Y);
		return 2;
	}
}

LuaWindow::~LuaWindow()
{
	if(ui::Engine::Ref().GetWindow() == window)
		ui::Engine::Ref().CloseWindow();
	delete window;
}