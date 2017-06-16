#include "Sign.h"
#include "graphics/Graphics.h"
#include "simulation/Simulation.h"

sign::sign(std::string text_, int x_, int y_, Justification justification_):
	x(x_),
	y(y_),
	ju(justification_),
	text(text_)
{
}

std::string sign::getText(Simulation *sim)
{
	char buff[256];
	char signText[256];
	sprintf(signText, "%s", text.substr(0, 255).c_str());

	if(signText[0] && signText[0] == '{')
	{
		if (!strcmp(signText,"{p}"))
		{
			float pressure = 0.0f;
			if (x>=0 && x<XRES && y>=0 && y<YRES)
				pressure = sim->pv[y/CELL][x/CELL];
			sprintf(buff, "Pressure: %3.2f", pressure);  //...pressure
		}
		else if (!strcmp(signText,"{aheat}"))
		{
			float aheat = 0.0f;
			if (x>=0 && x<XRES && y>=0 && y<YRES)
				aheat = sim->hv[y/CELL][x/CELL];
			sprintf(buff, "%3.2f", aheat-273.15);
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
			int pos = splitsign(signText);
			if (pos)
			{
				strcpy(buff, signText+pos+1);
				buff[strlen(signText)-pos-2]=0;
			}
			else
				strcpy(buff, signText);
		}
	}
	else
	{
		strcpy(buff, signText);
	}

	return std::string(buff);
}

void sign::pos(std::string signText, int & x0, int & y0, int & w, int & h)
{
	w = Graphics::textwidth(signText.c_str()) + 5;
	h = 15;
	x0 = (ju == Right) ? x - w :
		  (ju == Left) ? x : x - w/2;
	y0 = (y > 18) ? y - 18 : y + 4;
}

int sign::splitsign(const char* str, char * type)
{
	if (str[0]=='{' && (str[1]=='c' || str[1]=='t' || str[1]=='b' || str[1]=='s'))
	{
		const char* p = str+2;
		// signs with text arguments
		if (str[1] == 's')
		{
			if (str[2]==':')
			{
				p = str+4;
				while (*p && *p!='|')
					p++;
			}
			else
				return 0;
		}
		// signs with number arguments
		if (str[1] == 'c' || str[1] == 't')
		{
			if (str[2]==':' && str[3]>='0' && str[3]<='9')
			{
				p = str+4;
				while (*p>='0' && *p<='9')
					p++;
			}
			else
				return 0;
		}

		if (*p=='|')
		{
			int r = p-str;
			while (*p)
				p++;
			if (p[-1] == '}')
			{
				if (type)
					*type = str[1];
				return r;
			}
		}
	}
	return 0;
}
