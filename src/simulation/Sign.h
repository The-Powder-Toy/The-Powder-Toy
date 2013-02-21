//
//  Sign.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_Sign_h
#define The_Powder_Toy_Sign_h

#include <string>

class Simulation;

class sign
{
public:
	enum Justification { Left = 0, Centre = 1, Right = 2 };
	sign(std::string text_, int x_, int y_, Justification justification_);
	int x, y;
	Justification ju;
	std::string text;

	std::string getText(Simulation *sim);
	void pos(std::string signText, int & x0, int & y0, int & w, int & h);
};

#endif
