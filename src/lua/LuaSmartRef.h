#pragma once
#include "LuaCompat.h"
#include <utility>

class LuaSmartRef
{
	int ref = LUA_REFNIL;

public:
	LuaSmartRef() = default;
	LuaSmartRef(const LuaSmartRef &other) = delete;
	~LuaSmartRef();

	LuaSmartRef(LuaSmartRef &&other) noexcept : LuaSmartRef()
	{
		swap(*this, other);
	}

	LuaSmartRef &operator =(LuaSmartRef other)
	{
		swap(*this, other);
		return *this;
	}

	void Assign(lua_State *L, int index); // Copies the value before getting reference, stack unchanged.
	void Clear();
	int Push(lua_State *L); // Always pushes exactly one value, possibly nil.

	inline operator int() const
	{
		return ref;
	}

	inline operator bool() const
	{
		return ref != LUA_REFNIL;
	}

	friend void swap(LuaSmartRef &one, LuaSmartRef &other)
	{
		using std::swap;
		swap(one.ref, other.ref);
	}
};
