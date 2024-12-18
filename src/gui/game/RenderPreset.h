#pragma once
#include "common/String.h"
#include <cstdint>

struct RenderPreset
{
	String Name;
	uint32_t renderMode;
	uint32_t displayMode;
	uint32_t colorMode;
};
