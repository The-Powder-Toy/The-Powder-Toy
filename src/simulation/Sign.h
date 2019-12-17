#ifndef SIGN_H_
#define SIGN_H_
#include "Config.h"

#include "common/String.h"

#include <utility>

class Simulation;

struct sign
{
	enum Justification
	{
		Left = 0,
		Middle = 1,
		Right = 2,
		None = 3
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
	String getDisplayText(Simulation *sim, int &x, int &y, int &w, int &h, bool colorize = true, bool *v95 = nullptr);
	std::pair<int, Type> split();
};

#endif
