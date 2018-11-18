#include "LuaEvents.h"
#ifdef LUACONSOLE
#include "LuaCompat.h"
#include "LuaScriptInterface.h"

#include "Platform.h"
#include "gui/interface/Engine.h"
#endif

void Event::PushInteger(lua_State * l, int num)
{
#ifdef LUACONSOLE
	lua_pushinteger(l, num);
#endif
}

void Event::PushBoolean(lua_State * l, bool flag)
{
#ifdef LUACONSOLE
	lua_pushboolean(l, flag);
#endif
}

void Event::PushString(lua_State * l, ByteString str)
{
#ifdef LUACONSOLE
	lua_pushstring(l, str.c_str());
#endif
}

TextInputEvent::TextInputEvent(String text):
	text(text)
{}

int TextInputEvent::PushToStack(lua_State * l)
{
	PushString(l, text.ToUtf8());
	return 1;
}

KeyEvent::KeyEvent(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt):
	key(key),
	scan(scan),
	repeat(repeat),
	shift(shift),
	ctrl(ctrl),
	alt(alt)
{}

int KeyEvent::PushToStack(lua_State * l)
{
	PushInteger(l, key);
	PushInteger(l, scan);
	PushBoolean(l, repeat);
	PushBoolean(l, shift);
	PushBoolean(l, ctrl);
	PushBoolean(l, alt);

	return 6;
}

MouseDownEvent::MouseDownEvent(int x, int y, int button):
	x(x),
	y(y),
	button(button)
{}

int MouseDownEvent::PushToStack(lua_State * l)
{
	PushInteger(l, x);
	PushInteger(l, y);
	PushInteger(l, button);

	return 3;
}

MouseUpEvent::MouseUpEvent(int x, int y, int button, int reason):
	x(x),
	y(y),
	button(button),
	reason(reason)
{}

int MouseUpEvent::PushToStack(lua_State * l)
{
	PushInteger(l, x);
	PushInteger(l, y);
	PushInteger(l, button);
	PushInteger(l, reason);

	return 4;
}

MouseMoveEvent::MouseMoveEvent(int x, int y, int dx, int dy):
	x(x),
	y(y),
	dx(dx),
	dy(dy)
{}

int MouseMoveEvent::PushToStack(lua_State * l)
{
	PushInteger(l, x);
	PushInteger(l, y);
	PushInteger(l, dx);
	PushInteger(l, dy);

	return 4;
}

MouseWheelEvent::MouseWheelEvent(int x, int y, int d):
	x(x),
	y(y),
	d(d)
{}

int MouseWheelEvent::PushToStack(lua_State * l)
{
	PushInteger(l, x);
	PushInteger(l, y);
	PushInteger(l, d);

	return 3;
}

#ifdef LUACONSOLE
int LuaEvents::RegisterEventHook(lua_State *l, ByteString eventName)
{
	if (lua_isfunction(l, 2))
	{
		lua_pushstring(l, eventName.c_str());
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, 1);
			lua_newtable(l);
			lua_pushstring(l, eventName.c_str());
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int c = lua_objlen(l, -1);
		lua_pushvalue(l, 2);
		lua_rawseti(l, -2, c + 1);
	}
	lua_pushvalue(l, 2);
	return 1;
}

int LuaEvents::UnregisterEventHook(lua_State *l, ByteString eventName)
{
	if (lua_isfunction(l, 2))
	{
		lua_pushstring(l, eventName.c_str());
		lua_rawget(l, LUA_REGISTRYINDEX);
		if (!lua_istable(l, -1))
		{
			lua_pop(l, -1);
			lua_newtable(l);
			lua_pushstring(l, eventName.c_str());
			lua_pushvalue(l, -2);
			lua_rawset(l, LUA_REGISTRYINDEX);
		}
		int len = lua_objlen(l, -1);
		int adjust = 0;
		for (int i = 1; i <= len; i++)
		{
			lua_rawgeti(l, -1, i + adjust);
			// unregister the function
			if (lua_equal(l, 2, -1))
			{
				lua_pop(l, 1);
				adjust++;
				i--;
			}
			// Update the function index in the table if we've removed a previous function
			else if (adjust)
				lua_rawseti(l, -2, i);
			else
				lua_pop(l, 1);
		}
	}
	return 0;
}

bool LuaEvents::HandleEvent(LuaScriptInterface *luacon_ci, Event *event, ByteString eventName)
{
	ui::Engine::Ref().LastTick(Platform::GetTime());
	bool cont = true;
	lua_State* l = luacon_ci->l;
	lua_pushstring(l, eventName.c_str());
	lua_rawget(l, LUA_REGISTRYINDEX);
	if (!lua_istable(l, -1))
	{
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, eventName.c_str());
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_REGISTRYINDEX);
	}
	int len = lua_objlen(l, -1);
	for (int i = 1; i <= len && cont; i++)
	{
		lua_rawgeti(l, -1, i);
		int numArgs = event->PushToStack(l);
		int callret = lua_pcall(l, numArgs, 1, 0);
		if (callret)
		{
			if (luacon_geterror(luacon_ci) == "Error: Script not responding")
			{
				ui::Engine::Ref().LastTick(Platform::GetTime());
				for (int j = i; j <= len - 1; j++)
				{
					lua_rawgeti(l, -2, j + 1);
					lua_rawseti(l, -3, j);
				}
				lua_pushnil(l);
				lua_rawseti(l, -3, len);
				i--;
			}
			luacon_ci->Log(CommandInterface::LogError, luacon_geterror(luacon_ci));
			lua_pop(l, 1);
		}
		else
		{
			if (!lua_isnoneornil(l, -1))
				cont = lua_toboolean(l, -1);
			lua_pop(l, 1);
		}
		len = lua_objlen(l, -1);
	}
	lua_pop(l, 1);
	return cont;
}


String LuaEvents::luacon_geterror(LuaScriptInterface * luacon_ci)
{
	luaL_tostring(luacon_ci->l, -1);
	String err = ByteString(luaL_optstring(luacon_ci->l, -1, "failed to execute")).FromUtf8();
	lua_pop(luacon_ci->l, 1);
	return err;
}
#endif
