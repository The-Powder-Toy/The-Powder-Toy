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

String sign::getDisplayText(Simulation *sim, int &x0, int &y0, int &w, int &h, bool colorize, bool *v95)
{
	String drawable_text;
	auto si = std::make_pair(0, Type::Normal);
	if (text.find('{') == text.npos)
	{
		drawable_text = text;
	}
	else
	{
		si = split();
		if (si.first)
		{
			drawable_text = text.Between(si.first + 1, text.size() - 1);
		}
		else
		{
			Particle const *part = nullptr;
			float pressure = 0.0f;
			float aheat = 0.0f;
			if (sim && x >= 0 && x < XRES && y >= 0 && y < YRES)
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
					if (between_curlies == "t"_ascii || between_curlies == "temp"_ascii)
					{
						formatted_text << Format::Precision(Format::ShowPoint(part ? part->temp - 273.15f : 0.0f), 2);
						// * We would really only need to do this if the sign used the new
						//   keyword "temp" or if the text was more than just "{t}", but 95.0
						//   upgrades such signs at load time anyway.
						// * The same applies to "{p}" and "{aheat}" signs.
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "p"_ascii || between_curlies == "pres"_ascii)
					{
						formatted_text << Format::Precision(Format::ShowPoint(pressure), 2);
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "a"_ascii || between_curlies == "aheat"_ascii)
					{
						formatted_text << Format::Precision(Format::ShowPoint(aheat), 2);
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "type"_ascii)
					{
						formatted_text << (part ? sim->BasicParticleInfo(*part) : (formatted_text.Size() ? "empty"_i18n : "Empty"_i18n));
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "ctype"_ascii)
					{
						formatted_text << (part ? ((part->ctype && sim->IsValidElement(part->ctype)) ? sim->ElementResolve(part->ctype, -1) : String::Build(part->ctype)) : (formatted_text.Size() ? "empty"_i18n : "Empty"_i18n));
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "life"_ascii)
					{
						formatted_text << (part ? part->life : 0);
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "tmp"_ascii)
					{
						formatted_text << (part ? part->tmp : 0);
						if (v95)
							*v95 = true;
					}
					else if (between_curlies == "tmp2"_ascii)
					{
						formatted_text << (part ? part->tmp2 : 0);
						if (v95)
							*v95 = true;
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
			drawable_text = formatted_text.Build();
		}
	}

	if (colorize)
	{
		switch (si.second)
		{
		case Normal: break;
		case Save:   drawable_text = "\bt"_ascii + drawable_text; break;
		case Thread: drawable_text = "\bl"_ascii + drawable_text; break;
		case Button: drawable_text = "\bo"_ascii + drawable_text; break;
		case Search: drawable_text = "\bu"_ascii + drawable_text; break;
		}
	}

	w = Graphics::textwidth(drawable_text.c_str()) + 5;
	h = 15;
	x0 = (ju == Right) ? x - w : (ju == Left) ? x : x - w/2;
	y0 = (y > 18) ? y - 18 : y + 4;

	return drawable_text;
}

std::pair<int, sign::Type> sign::split()
{
	String::size_type pipe = 0;
	if (text.size() >= 4 && text.front() == '{' && text.back() == '}')
	{
		switch (text[1])
		{
		case 'c':
		case 't':
			if (text[2] == ':' && (pipe = text.find('|', 4)) != text.npos)
			{
				for (String::size_type i = 3; i < pipe; ++i)
				{
					if (text[i] < '0' || text[i] > '9')
					{
						return std::make_pair(0, Type::Normal);
					}
				}
				return std::make_pair(pipe, text[1] == 'c' ? Type::Save : Type::Thread);
			}
			break;

		case 'b':
			if (text[2] == '|')
			{
				return std::make_pair(2, Type::Button);
			}
			break;

		case 's':
			if (text[2] == ':' && (pipe = text.find('|', 3)) != text.npos)
			{
				return std::make_pair(pipe, Type::Search);
			}
			break;
		}
	}
	return std::make_pair(0, Type::Normal);
}
