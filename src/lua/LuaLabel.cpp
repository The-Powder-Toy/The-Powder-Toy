#include "LuaScriptInterface.h"
#include "LuaLabel.h"
#include "gui/interface/Label.h"

const char LuaLabel::className[] = "label";

#define method(class, name) {#name, &class::name}
Luna<LuaLabel>::RegType LuaLabel::methods[] = {
	method(LuaLabel, text),
	method(LuaLabel, position),
	method(LuaLabel, size),
	method(LuaLabel, visible),
	{nullptr, nullptr}
};

LuaLabel::LuaLabel(lua_State *L) :
	LuaComponent(L)
{
	this->L = L;
	int posX = luaL_optinteger(L, 1, 0);
	int posY = luaL_optinteger(L, 2, 0);
	int sizeX = luaL_optinteger(L, 3, 10);
	int sizeY = luaL_optinteger(L, 4, 10);
	String text = tpt_lua_optString(L, 5, "");

	label = new ui::Label(ui::Point(posX, posY), ui::Point(sizeX, sizeY), text);
	component = label;
}

int LuaLabel::text(lua_State *L)
{
	int args = lua_gettop(L);
	if(args)
	{
		label->SetText(tpt_lua_checkString(L, 1));
		return 0;
	}
	else
	{
		tpt_lua_pushString(L, label->GetText());
		return 1;
	}
}

LuaLabel::~LuaLabel()
{
}
