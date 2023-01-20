#pragma once
#include "common/String.h"
#include <variant>

struct TextInputEvent
{
	String text;
};

struct TextEditingEvent
{
	String text;
};

struct KeyEvent
{
	int key;
	int scan;
	bool repeat;
	bool shift;
	bool ctrl;
	bool alt;
};

struct KeyPressEvent : public KeyEvent
{
};

struct KeyReleaseEvent : public KeyEvent
{
};

struct MouseDownEvent
{
	int x;
	int y;
	unsigned int button;
};

struct MouseUpEvent
{
	int x;
	int y;
	unsigned int button;
	int reason;
};

struct MouseMoveEvent
{
	int x;
	int y;
	int dx;
	int dy;
};

struct MouseWheelEvent
{
	int x;
	int y;
	int d;
};

struct TickEvent
{
};

struct BlurEvent
{
};

struct CloseEvent
{
};

struct BeforeSimEvent
{
};

struct AfterSimEvent
{
};

using GameControllerEvent = std::variant<
	TextInputEvent,
	TextEditingEvent,
	KeyPressEvent,
	KeyReleaseEvent,
	MouseDownEvent,
	MouseUpEvent,
	MouseMoveEvent,
	MouseWheelEvent,
	TickEvent,
	BlurEvent,
	CloseEvent,
	BeforeSimEvent,
	AfterSimEvent
>;
