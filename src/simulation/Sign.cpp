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

String sign::getText(Simulation *sim)
{
	if (text.find('{') == text.npos)
	{
		return text;
	}

	if (int pos = splitsign(text))
	{
		return text.Between(pos + 1, text.size() - 1);
	}

	Particle const *part = nullptr;
	float pressure = 0.0f;
	float aheat = 0.0f;
	if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		if (sim->photons[y][x])
		{
			part = &(sim->parts[ID(sim->photons[y][x])]);
		}
		else if (sim->pmap[y][x])
		{
			part = &(sim->parts[ID(sim->pmap[y][x])]);
		}
		pressure = sim->pv[y/CELL][x/CELL];
		aheat = sim->hv[y/CELL][x/CELL] - 273.15f;
	}

	String remaining_text = text;
	StringBuilder formatted_text;
	while (auto split_left_curly = remaining_text.SplitBy('{'))
	{
		String after_left_curly = split_left_curly.After();
		if (auto split_right_curly = after_left_curly.SplitBy('}'))
		{
			formatted_text << split_left_curly.Before();
			remaining_text = split_right_curly.After();
			String between_curlies = split_right_curly.Before();
			if (between_curlies == "t" || between_curlies == "temp")
			{
				formatted_text << Format::Precision(Format::ShowPoint(part ? part->temp - 273.15f : 0.0f), 2);
			}
			else if (between_curlies == "p" || between_curlies == "pres")
			{
				formatted_text << Format::Precision(Format::ShowPoint(pressure), 2);
			}
			else if (between_curlies == "a" || between_curlies == "aheat")
			{
				formatted_text << Format::Precision(Format::ShowPoint(aheat), 2);
			}
			else if (between_curlies == "type")
			{
				formatted_text << (part ? sim->BasicParticleInfo(*part) : (formatted_text.Size() ? String::Build("empty") : String::Build("Empty")));
			}
			else if (between_curlies == "ctype")
			{
				formatted_text << (part ? ((part->ctype && sim->IsValidElement(part->ctype)) ? sim->ElementResolve(part->ctype, -1) : String::Build(part->ctype)) : (formatted_text.Size() ? String::Build("empty") : String::Build("Empty")));
			}
			else if (between_curlies == "life")
			{
				formatted_text << (part ? part->life : 0);
			}
			else if (between_curlies == "tmp")
			{
				formatted_text << (part ? part->tmp : 0);
			}
			else if (between_curlies == "tmp2")
			{
				formatted_text << (part ? part->tmp2 : 0);
			}
			else
			{
				formatted_text << '{' << between_curlies << '}';
			}
		}
		else
		{
			break;
		}
	}
	formatted_text << remaining_text;
	return formatted_text.Build();
}
