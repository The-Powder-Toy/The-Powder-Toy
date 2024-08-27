#include "LuaProgressBar.h"
#include "LuaScriptInterface.h"
#include "gui/interface/ProgressBar.h"

const char LuaProgressBar::className[] = "progressBar";

#define method(class, name) {#name, &class::name}
Luna<LuaProgressBar>::RegType LuaProgressBar::methods[] = {
	method(LuaProgressBar, position),
	method(LuaProgressBar, size),
	method(LuaProgressBar, visible),
	method(LuaProgressBar, progress),
	method(LuaProgressBar, status),
	{0, 0}
};

LuaProgressBar::LuaProgressBar(lua_State *L) :
	LuaComponent(L)
{
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	int value = luaL_optinteger(L, 5, 0);
	String status = tpt_lua_optString(L, 6, "");

	progressBar = new ui::ProgressBar(ui::Point(posX, posY), ui::Point(sizeX, sizeY), value, status);
	component = progressBar;
}

int LuaProgressBar::progress(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		progressBar->SetProgress(lua_tointeger(L, 1));
		return 0;
	}
	else
	{
		lua_pushinteger(L, progressBar->GetProgress());
		return 1;
	}
}

int LuaProgressBar::status(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		progressBar->SetStatus(tpt_lua_checkString(L, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(L, progressBar->GetStatus());
		return 1;
	}
}

LuaProgressBar::~LuaProgressBar()
{
}
