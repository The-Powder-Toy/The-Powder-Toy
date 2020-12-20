#ifndef LUAEVENTS_H
#define LUAEVENTS_H
#include "Config.h"

#include "common/String.h"

struct lua_State;
class LuaScriptInterface;

class Event
{
protected:
	void PushInteger(lua_State * l, int num);
	void PushBoolean(lua_State * l, bool flag);
	void PushString(lua_State * l, ByteString str);

public:
	virtual int PushToStack(lua_State * l) = 0;
	virtual ~Event() = default;
};

class TextInputEvent : public Event
{
	String text;

public:
	TextInputEvent(String text);

	int PushToStack(lua_State * l) override;
};

class KeyEvent : public Event
{
	int key;
	int scan;
	bool repeat;
	bool shift;
	bool ctrl;
	bool alt;

public:
	KeyEvent(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);

	int PushToStack(lua_State * l) override;
};

class MouseDownEvent : public Event
{
	int x;
	int y;
	int button;

public:
	MouseDownEvent(int x, int y, int button);

	int PushToStack(lua_State * l) override;
};

class MouseUpEvent : public Event
{
	int x;
	int y;
	int button;
	int reason;

public:
	MouseUpEvent(int x, int y, int button, int reason);

	int PushToStack(lua_State * l) override;
};

class MouseMoveEvent : public Event
{
	int x;
	int y;
	int dx;
	int dy;

public:
	MouseMoveEvent(int x, int y, int dx, int dy);

	int PushToStack(lua_State * l) override;
};

class MouseWheelEvent : public Event
{
	int x;
	int y;
	int d;

public:
	MouseWheelEvent(int x, int y, int d);

	int PushToStack(lua_State * l) override;
};

class TickEvent: public Event
{
public:
	int PushToStack(lua_State *l) override { return 0; }
};

class BlurEvent: public Event
{
public:
	int PushToStack(lua_State *l) override { return 0; }
};

class CloseEvent: public Event
{
public:
	int PushToStack(lua_State *l) override { return 0; }
};

class LuaEvents
{
public:
	enum EventTypes {
		keypress,
		keyrelease,
		textinput,
		mousedown,
		mouseup,
		mousemove,
		mousewheel,
		tick,
		blur,
		close
	};

	static int RegisterEventHook(lua_State *l, ByteString eventName);
	static int UnregisterEventHook(lua_State *l, ByteString eventName);
	static bool HandleEvent(LuaScriptInterface *luacon_ci, Event *event, ByteString eventName);

	static String luacon_geterror(LuaScriptInterface *luacon_ci);
};

#endif // LUAEVENTS_H
