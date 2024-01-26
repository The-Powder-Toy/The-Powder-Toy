#pragma once
#include "common/String.h"
#include <utility>

class Simulation;

struct sign
{
	enum Justification
	{
		Left,
		Middle,
		Right,
		None,
		Max,
	};

	enum Type
	{
		Normal,
		Save,
		Thread,
		Button,
		Search
	};

	int x, y;
	Justification ju;
	String text;

	sign(String text_, int x_, int y_, Justification justification_);
	String getDisplayText(const Simulation *sim, int &x, int &y, int &w, int &h, bool colorize = true, bool *v95 = nullptr) const;
	std::pair<int, Type> split() const;
};
