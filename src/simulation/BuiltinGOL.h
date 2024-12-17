#pragma once

#include "graphics/Pixel.h"
#include "common/String.h"

struct BuiltinGOL
{
	String name;
	int oldtype;
	int ruleset;
	RGB colour = RGB(0, 0, 0);
	RGB colour2 = RGB(0, 0, 0);
	int goltype;
	String description;
};
