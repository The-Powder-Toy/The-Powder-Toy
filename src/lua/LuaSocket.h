#pragma once
#include "LuaCompat.h"

namespace LuaSocket
{
	double Now();
	void Timeout(double timeout);
	void Open(lua_State *l);
	void OpenTCP(lua_State *l);
}
