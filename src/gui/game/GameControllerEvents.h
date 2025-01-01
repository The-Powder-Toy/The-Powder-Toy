#pragma once
#include "common/String.h"
#include <variant>
#include <cstdint>

enum EventTraits : uint32_t
{
	eventTraitNone        = UINT32_C(0x00000000),
	eventTraitSimRng      = UINT32_C(0x00000001),
	eventTraitSimGraphics = UINT32_C(0x00000002),
	eventTraitHindersSrt  = UINT32_C(0x00000004),
};
constexpr EventTraits operator |(EventTraits lhs, EventTraits rhs)
{
    return static_cast<EventTraits>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

struct TextInputEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	String text;
};

struct TextEditingEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	String text;
};

struct KeyEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	int key;
	int scan;
	bool repeat;
	bool shift;
	bool ctrl;
	bool alt;
};

struct KeyPressEvent : public KeyEvent
{
	static constexpr EventTraits traits = eventTraitNone;
};

struct KeyReleaseEvent : public KeyEvent
{
	static constexpr EventTraits traits = eventTraitNone;
};

struct MouseDownEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	int x;
	int y;
	unsigned int button;
};

struct MouseUpEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	int x;
	int y;
	unsigned int button;
	int reason;
};

struct MouseMoveEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	int x;
	int y;
	int dx;
	int dy;
};

struct MouseWheelEvent
{
	static constexpr EventTraits traits = eventTraitNone;
	int x;
	int y;
	int d;
};

struct TickEvent
{
	static constexpr EventTraits traits = eventTraitNone;
};

struct BlurEvent
{
	static constexpr EventTraits traits = eventTraitNone;
};

struct CloseEvent
{
	static constexpr EventTraits traits = eventTraitNone;
};

struct BeforeSimEvent
{
	static constexpr EventTraits traits = eventTraitSimRng;
};

struct AfterSimEvent
{
	static constexpr EventTraits traits = eventTraitSimRng;
};

struct BeforeSimDrawEvent
{
	static constexpr EventTraits traits = eventTraitSimGraphics | eventTraitHindersSrt;
};

struct AfterSimDrawEvent
{
	static constexpr EventTraits traits = eventTraitSimGraphics;
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
	AfterSimEvent,
	BeforeSimDrawEvent,
	AfterSimDrawEvent
>;
