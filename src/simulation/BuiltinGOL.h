#pragma once

#include "graphics/Pixel.h"
#include "common/String.h"

struct BuiltinGOL
{
	String name;
	int oldtype;
	int ruleset;
	RGB<uint8_t> colour, colour2;
	int goltype;
	String description;
};
