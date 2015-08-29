#ifndef SIGN_H_
#define SIGN_H_

#include <string>

class Simulation;

class sign
{
public:
	enum Justification { Left = 0, Middle = 1, Right = 2, None = 3 };
	sign(std::string text_, int x_, int y_, Justification justification_);
	int x, y;
	Justification ju;
	std::string text;

	std::string getText(Simulation *sim);
	void pos(std::string signText, int & x0, int & y0, int & w, int & h);

	static int splitsign(const char* str, char * type = NULL);
};

#endif
