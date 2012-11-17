/*
 * Sign.cpp
 *
 *  Created on: Jun 25, 2012
 *      Author: Simon
 */

#include "Sign.h"
#include "graphics/Graphics.h"
#include "Misc.h"

sign::sign(std::string text_, int x_, int y_, Justification justification_):
	text(text_),
	x(x_),
	y(y_),
	ju(justification_)
{
}

void sign::pos(int & x0, int & y0, int & w, int & h)
{
	//Changing width if sign have special content
	if (text == "{p}")
	{
		w = Graphics::textwidth("Pressure: -000.00");
	}
	else if (text == "{t}")
	{
		w = Graphics::textwidth("Temp: 0000.00");
	}
	else if (sregexp(text.c_str(), "^{[c|t]:[0-9]*|.*}$")==0)
	{
		int sldr, startm;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		for (sldr=3; text[sldr-1] != '|'; sldr++)
			startm = sldr + 1;

		sldr = startm;
		while (text[sldr] != '}')
		{
			buff[sldr - startm] = text[sldr];
			sldr++;
		}
		w = Graphics::textwidth(buff) + 5;
	}
	else
	{
		w = Graphics::textwidth(text.c_str()) + 5;
	}
	h = 14;
	x0 = (ju == 2) ? x - w :
	      (ju == 1) ? x - w/2 : x;
	y0 = (y > 18) ? y - 18 : y + 4;
}
