#pragma once

#include "LuaCompat.h"

class LuaSmartRef
{
	int ref;

protected:
	lua_State *l;

public:
	LuaSmartRef(lua_State *l);
	~LuaSmartRef();
	void Assign(int index); // Copies the value before getting reference, stack unchanged.
	void Clear();
	operator int() const;
	operator bool() const;
};

