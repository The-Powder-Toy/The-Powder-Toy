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
