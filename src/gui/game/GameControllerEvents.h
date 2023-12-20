#pragma once
#include "common/String.h"
#include <variant>

struct TextInputEvent
{
	static constexpr bool simEvent = false;
	String text;
};

struct TextEditingEvent
{
	static constexpr bool simEvent = false;
	String text;
};

struct KeyEvent
{
	static constexpr bool simEvent = false;
	int key;
	int scan;
	bool repeat;
	bool shift;
	bool ctrl;
	bool alt;
};

struct KeyPressEvent : public KeyEvent
{
	static constexpr bool simEvent = false;
};

struct KeyReleaseEvent : public KeyEvent
{
	static constexpr bool simEvent = false;
};

struct MouseDownEvent
{
	static constexpr bool simEvent = false;
	int x;
	int y;
	unsigned int button;
};

struct MouseUpEvent
{
	static constexpr bool simEvent = false;
	int x;
	int y;
	unsigned int button;
	int reason;
};

struct MouseMoveEvent
{
	static constexpr bool simEvent = false;
	int x;
	int y;
	int dx;
	int dy;
};

struct MouseWheelEvent
{
	static constexpr bool simEvent = false;
	int x;
	int y;
	int d;
};

struct TickEvent
{
	static constexpr bool simEvent = false;
};

struct BlurEvent
{
	static constexpr bool simEvent = false;
};

struct CloseEvent
{
	static constexpr bool simEvent = false;
};

struct BeforeSimEvent
{
	static constexpr bool simEvent = true;
};

struct AfterSimEvent
{
	static constexpr bool simEvent = true;
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
