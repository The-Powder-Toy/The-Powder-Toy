//
//  Sign.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_Sign_h
#define The_Powder_Toy_Sign_h

class sign
{
public:
	enum Justification { Left = 0, Centre = 1, Right = 2 };
	sign(std::string text_, int x_, int y_, Justification justification_):
	text(text_),
	x(x_),
	y(y_),
	ju(justification_)
	{}
	int x, y;
	Justification ju;
	std::string text;
};

#endif
