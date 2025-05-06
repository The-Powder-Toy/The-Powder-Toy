#pragma once
#include "common/String.h"
#include <variant>
#include <cstdint>

enum EventTraits : uint32_t
{
	eventTraitNone              = UINT32_C(0x00000000),
	eventTraitSimRng            = UINT32_C(0x00000001),
	eventTraitSimGraphics       = UINT32_C(0x00000002),
	eventTraitHindersSrt        = UINT32_C(0x00000004),
	eventTraitInterface         = UINT32_C(0x00000008),
	eventTraitInterfaceGraphics = UINT32_C(0x00000010),
	eventTraitConstSim          = UINT32_C(0x00000020),
	eventTraitConstTools        = UINT32_C(0x00000040),
	eventTraitMonopartAccess    = UINT32_C(0x00000080),
};
constexpr EventTraits operator |(EventTraits lhs, EventTraits rhs)
{
    return static_cast<EventTraits>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

struct TextInputEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	String text;
};

struct TextEditingEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	String text;
};

struct KeyEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	int key;
	int scan;
	bool repeat;
	bool shift;
	bool ctrl;
	bool alt;
};

struct KeyPressEvent : public KeyEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
};

struct KeyReleaseEvent : public KeyEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
};

struct MouseDownEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	int x;
	int y;
	unsigned int button;
};

struct MouseUpEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	int x;
	int y;
	unsigned int button;
	int reason;
};

struct MouseMoveEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	int x;
	int y;
	int dx;
	int dy;
};

struct MouseWheelEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
	int x;
	int y;
	int d;
};

struct TickEvent
{
	static constexpr EventTraits traits = eventTraitInterface | eventTraitInterfaceGraphics;
};

struct BlurEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
};

struct CloseEvent
{
	static constexpr EventTraits traits = eventTraitInterface;
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
	static constexpr EventTraits traits = eventTraitSimGraphics | eventTraitHindersSrt | eventTraitInterface | eventTraitConstSim;
};

struct AfterSimDrawEvent
{
	static constexpr EventTraits traits = eventTraitSimGraphics | eventTraitInterface | eventTraitConstSim;
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
