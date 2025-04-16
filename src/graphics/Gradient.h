#pragma once
#include "Pixel.h"
#include <vector>

struct GradientStop
{
	RGB color;
	float point;

	bool operator <(const GradientStop &other) const
	{
		return point < other.point;
	}
};
std::vector<RGB> Gradient(std::vector<GradientStop> stops, int resolution);
