#pragma once
#include <variant>

struct FpsLimitNone
{
};
struct FpsLimitExplicit
{
	using ValueType = float;
	static constexpr ValueType minSane = 3.f;
	static constexpr ValueType maxSane = 1000.f;
	ValueType value;
};
using SimFpsLimit = std::variant<FpsLimitNone, FpsLimitExplicit>;
struct FpsLimitFollowDraw
{
};
using FpsLimit = std::variant<FpsLimitNone, FpsLimitExplicit, FpsLimitFollowDraw>;

struct DrawLimitDisplay
{
};
struct DrawLimitExplicit
{
	using ValueType = int;
	static constexpr ValueType minSane = 1;
	static constexpr ValueType maxSane = 1000;
	ValueType value;
};
// TODO: DrawLimitVsync
using DrawLimit = std::variant<DrawLimitDisplay, DrawLimitExplicit>;

constexpr auto DefaultFpsLimit = FpsLimitExplicit{ 60.f };
constexpr auto DefaultDrawLimit = DrawLimitExplicit{ 60 };
