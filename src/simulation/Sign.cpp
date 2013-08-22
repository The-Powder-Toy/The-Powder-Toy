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
	else
	{
		int match=0;
		const char* r;
		const char* e;
		if (signText[0]=='{' && (signText[1]=='c' || signText[1]=='t') && signText[2]==':' && signText[3]>='0' && signText[3]<='9')
		{
			const char* p=signText+4;
			while (*p>='0' && *p<='9')
				p++;
			if (*p=='|')
			{
				r=p+1;
				while (*p)
					p++;
				if (p[-1]=='}')
				{
					match=1;
					e=p;
				}
			}
		}
		if (match)
		{
			strcpy(buff, r);
			buff[e-r-1]=0;
		}
		else
			strcpy(buff, signText);
	}

	return std::string(buff);
}

void sign::pos(std::string signText, int & x0, int & y0, int & w, int & h)
{
	w = Graphics::textwidth(signText.c_str()) + 5;
	h = 15;
	x0 = (ju == 2) ? x - w :
	      (ju == 1) ? x - w/2 : x;
	y0 = (y > 18) ? y - 18 : y + 4;
}
