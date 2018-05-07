#ifndef SIGN_H_
#define SIGN_H_

#include "common/String.h"

class Simulation;

class sign
{
public:
	enum Justification { Left = 0, Middle = 1, Right = 2, None = 3 };
	sign(String text_, int x_, int y_, Justification justification_);
	int x, y;
	Justification ju;
	String text;

	String getText(Simulation *sim);
	void pos(String signText, int & x0, int & y0, int & w, int & h);

	static int splitsign(String str, String::value_type *type = NULL);
};

#endif
