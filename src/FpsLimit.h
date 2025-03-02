#pragma once
#include <variant>

struct FpsLimitNone
{
};
struct FpsLimitExplicit
{
	float value;
};
using SimFpsLimit = std::variant<FpsLimitNone, FpsLimitExplicit>;
struct FpsLimitFollowDraw
{
};
using FpsLimit = std::variant<FpsLimitNone, FpsLimitExplicit, FpsLimitFollowDraw>;

struct DrawLimitDisplay
{
};
struct DrawLimitNone
{
};
struct DrawLimitExplicit
{
	int value;
};
// TODO: DrawLimitVsync
using DrawLimit = std::variant<DrawLimitDisplay, DrawLimitNone, DrawLimitExplicit>;
