extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <iostream>
#include "LuaLabel.h"
#include "interface/Label.h"

const char LuaLabel::className[] = "Label";

#define method(class, name) {#name, &class::name}
Luna<LuaLabel>::RegType LuaLabel::methods[] = {
	method(LuaLabel, text),
	method(LuaLabel, position),
	method(LuaLabel, size),
	{0, 0}
};

LuaLabel::LuaLabel(lua_State * l)
{
	this->l = l;
	int posX = luaL_optinteger(l, 1, 0);
	int posY = luaL_optinteger(l, 2, 0);
	int sizeX = luaL_optinteger(l, 3, 10);
	int sizeY = luaL_optinteger(l, 4, 10);
	std::string text = luaL_optstring(l, 5, "");

	label = new ui::Label(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text);
}

int LuaLabel::text(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TSTRING);
		label->SetText(lua_tostring(l, 1));
		return 0;
	}
	else
	{
		lua_pushstring(l, label->GetText().c_str());
		return 1;
	}
}

int LuaLabel::position(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		label->Position = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		return 0;
	}
	else
	{
		lua_pushinteger(l, label->Position.X);
		lua_pushinteger(l, label->Position.Y);
		return 2;
	}
}

int LuaLabel::size(lua_State * l)
{
	int args = lua_gettop(l);
	if(args)
	{
		luaL_checktype(l, 1, LUA_TNUMBER);
		luaL_checktype(l, 2, LUA_TNUMBER);
		label->Size = ui::Point(lua_tointeger(l, 1), lua_tointeger(l, 2));
		label->Invalidate();
		return 0;
	}
	else
	{
		lua_pushinteger(l, label->Size.X);
		lua_pushinteger(l, label->Size.Y);
		return 2;
	}
}

LuaLabel::~LuaLabel()
{
	if(label->GetParentWindow())
		label->GetParentWindow()->RemoveComponent(label);
	delete label;
}