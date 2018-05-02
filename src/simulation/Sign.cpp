#include <iomanip>
#include "Sign.h"
#include "graphics/Graphics.h"
#include "simulation/Simulation.h"

sign::sign(String text_, int x_, int y_, Justification justification_):
	x(x_),
	y(y_),
	ju(justification_),
	text(text_)
{
}

String sign::getText(Simulation *sim)
{
	if (text[0] && text[0] == '{')
	{
		if (text == "{p}")
		{
			float pressure = 0.0f;
			if (x >= 0 && x < XRES && y >= 0 && y < YRES)
				pressure = sim->pv[y/CELL][x/CELL];
			return String::Build("Pressure: ", Format::Precision(Format::ShowPoint(pressure), 2));
		}
		else if (text == "{aheat}")
		{
			float aheat = 0.0f;
			if (x >= 0 && x < XRES && y >= 0 && y < YRES)
				aheat = sim->hv[y/CELL][x/CELL];
			return String::Build(Format::Precision(Format::ShowPoint(aheat - 273.15f), 2));
		}
		else if (text == "{t}")
		{
			if (x >= 0 && x < XRES && y >= 0 && y < YRES && sim->pmap[y][x])
				return String::Build("Temp: ", Format::Precision(Format::ShowPoint(sim->parts[ID(sim->pmap[y][x])].temp - 273.15f), 2));
			else
				return String::Build("Temp: ", Format::Precision(Format::ShowPoint(0), 2));
		}
		else
		{
			int pos = splitsign(text);
			if (pos)
				return text.Between(pos + 1, text.size() - 1);
			else
				return text;
		}
	}
	else
	{
		return text;
	}
}

void sign::pos(String signText, int & x0, int & y0, int & w, int & h)
{
	w = Graphics::textwidth(signText.c_str()) + 5;
	h = 15;
	x0 = (ju == Right) ? x - w :
		  (ju == Left) ? x : x - w/2;
	y0 = (y > 18) ? y - 18 : y + 4;
}

int sign::splitsign(String str, String::value_type *type)
{
	if (str[0] == '{' && (str[1] == 'c' || str[1] == 't' || str[1] == 'b' || str[1] == 's'))
	{
		size_t strIndex = 2;
		// Signs with text arguments
		if (str[1] == 's')
		{
			if (str[2] == ':')
			{
				strIndex = 3;
				while (strIndex < str.length() && str[strIndex] != '|')
					strIndex++;
			}
			else
				return 0;
		}
		// Signs with number arguments
		if (str[1] == 'c' || str[1] == 't')
		{
			if (str[2] == ':' && str[3] >= '0' && str[3] <= '9')
			{
				strIndex = 4;
				while (str[strIndex] >= '0' && str[strIndex] <= '9')
					strIndex++;
			}
			else
				return 0;
		}

		if (str[strIndex] == '|')
		{
			if (str[str.length() - 1] == '}')
			{
				if (type)
					*type = str[1];
				return strIndex;
			}
		}
	}
	return 0;
}
