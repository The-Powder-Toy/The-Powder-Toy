#include "Sign.h"
#include "graphics/Graphics.h"
#include "simulation/Simulation.h"
#include "Misc.h"

sign::sign(std::string text_, int x_, int y_, Justification justification_):
	text(text_),
	x(x_),
	y(y_),
	ju(justification_)
{
}

std::string sign::getText(Simulation *sim)
{
	char buff[256];
	char signText[256];
	sprintf(signText, "%s", text.substr(0, 255).c_str());

	if (!strcmp(signText,"{p}"))
	{
		float pressure = 0.0f;
		if (x>=0 && x<XRES && y>=0 && y<YRES)
			pressure = sim->pv[y/CELL][x/CELL];
		sprintf(buff, "Pressure: %3.2f", pressure);  //...pressure
	}
	else if (!strcmp(signText,"{t}"))
	{
		if (x>=0 && x<XRES && y>=0 && y<YRES && sim->pmap[y][x])
			sprintf(buff, "Temp: %4.2f", sim->parts[sim->pmap[y][x]>>8].temp-273.15);  //...temperature
		else
			sprintf(buff, "Temp: 0.00");  //...temperature
	}
	else if (sregexp(signText, "^{[c|t]:[0-9]*|.*}$")==0)
	{
		int sldr, startm;
		memset(buff, 0, sizeof(buff));
		for (sldr=3; signText[sldr-1] != '|'; sldr++)
			startm = sldr + 1;
		sldr = startm;
		while (signText[sldr] != '}')
		{
			buff[sldr - startm] = signText[sldr];
			sldr++;
		}
	}
	else
	{
		sprintf(buff, "%s", signText);
	}

	return std::string(buff,256);
}

void sign::pos(std::string signText, int & x0, int & y0, int & w, int & h)
{
	w = Graphics::textwidth(signText.c_str()) + 5;
	h = 15;
	x0 = (ju == 2) ? x - w :
	      (ju == 1) ? x - w/2 : x;
	y0 = (y > 18) ? y - 18 : y + 4;
}
