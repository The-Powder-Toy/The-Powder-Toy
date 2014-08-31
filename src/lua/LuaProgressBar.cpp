#ifdef LUACONSOLE

#include <iostream>
#include "LuaProgressBar.h"
#include "LuaScriptInterface.h"
#include "gui/interface/ProgressBar.h"

const char LuaProgressBar::className[] = "ProgressBar";

#define method(class, name) {#name, &class::name}
Luna<LuaProgressBar>::RegType LuaProgressBar::methods[] = {
	method(LuaProgressBar, position),
	method(LuaProgressBar, size),
	method(LuaProgressBar, visible),
	method(LuaProgressBar, progress),
	method(LuaProgressBar, status),
	{0, 0}
};

LuaProgressBar::LuaProgressBar(lua_State * l) :
	LuaComponent(l)
{
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	int value = luaL_optinteger(l, 5, 0);
	std::string status = luaL_optstring(l, 6, "");

	progressBar = new ui::ProgressBar(ui::Point(posX, posY), ui::Point(sizeX, sizeY), value, status);
	component = progressBar;
}

int LuaProgressBar::progress(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		progressBar->SetProgress(lua_tointeger(l, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(l, progressBar->GetProgress());
		return 1;
	}
}

int LuaProgressBar::status(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		progressBar->SetStatus(std::string(lua_tostring(l, 1)));
		return 0;
	}
	else
	{
		lua_pushstring(l, progressBar->GetStatus().c_str());
		return 1;
	}
}

LuaProgressBar::~LuaProgressBar()
{
}
#endif
