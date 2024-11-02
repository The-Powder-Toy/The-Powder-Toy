#pragma once
#include "graphics/Pixel.h"
#include "common/String.h"

struct CustomGOLData
{
	int rule;
	RGB colour1, colour2;
	String nameString;

	inline bool operator <(const CustomGOLData &other) const
	{
		return rule < other.rule;
	}
};
