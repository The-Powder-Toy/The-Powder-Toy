#pragma once
#include <variant>

struct FpsLimitVsync
{
};
struct FpsLimitNone
{
};
struct FpsLimitExplicit
{
	float value;
};
using FpsLimit = std::variant<FpsLimitVsync, FpsLimitNone, FpsLimitExplicit>;

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
using DrawLimit = std::variant<DrawLimitDisplay, DrawLimitNone, DrawLimitExplicit>;
