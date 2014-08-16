#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DRAY PT_DRAY 178
Element_DRAY::Element_DRAY()
{
	Identifier = "DEFAULT_PT_DRAY";
	Name = "DRAY";
	Colour = PIXPACK(0xFFAA22);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 0;
	Description = "Duplicator ray. Replicates a line of particles in front of it.";

	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DRAY::update;
	Graphics = NULL;
}

//should probably be in Simulation.h
bool InBounds(int x, int y)
{
	return (x>=0 && y>=0 && x<XRES && y<YRES);
}

//#TPT-Directive ElementHeader Element_DRAY static int update(UPDATE_FUNC_ARGS)
int Element_DRAY::update(UPDATE_FUNC_ARGS)
{
	int ctype = parts[i].ctype&0xFF, ctypeExtra = parts[i].ctype>>8, copyLength = parts[i].tmp, copySpaces = parts[i].tmp2;
	if (copySpaces <= 0)
		copySpaces = 0;
	if (copyLength <= 0)
		copyLength = 0;
	else
		copySpaces++; //strange hack
	if (!parts[i].life) // only fire when life is 0, but nothing sets the life right now
	{
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3) //spark found, start creating
					{
						bool overwrite = parts[r>>8].ctype == PT_PSCN;
						int partsRemaining = copyLength, xCopyTo, yCopyTo; //positions where the line will start being copied at

						if (parts[r>>8].ctype == PT_INWR && rx && ry) // INWR doesn't spark from diagonals
							continue;

						//figure out where the copying will start/end
						for (int xStep = rx*-1, yStep = ry*-1, xCurrent = x+xStep, yCurrent = y+yStep; ; xCurrent+=xStep, yCurrent+=yStep)
						{
							int rr = pmap[yCurrent][xCurrent];
							if ((!copyLength && (rr&0xFF) == ctype && (ctype != PT_LIFE || parts[rr>>8].ctype == ctypeExtra))
									|| !(--partsRemaining && InBounds(xCurrent+xStep, yCurrent+yStep)))
							{
								copyLength -= partsRemaining;
								xCopyTo = xCurrent + xStep*copySpaces;
								yCopyTo = yCurrent + yStep*copySpaces;
								break;
							}
						}

						//now, actually copy the particles
						partsRemaining = copyLength + 1;
						for (int xStep = rx*-1, yStep = ry*-1, xCurrent = x+xStep, yCurrent = y+yStep; InBounds(xCopyTo, yCopyTo) && --partsRemaining; xCurrent+=xStep, yCurrent+=yStep, xCopyTo+=xStep, yCopyTo+=yStep)
						{
							int type = pmap[yCurrent][xCurrent]&0xFF, p;
							if (overwrite)
								sim->delete_part(xCopyTo, yCopyTo);
							if (type == PT_SPRK) //hack
								p = sim->create_part(-1, xCopyTo, yCopyTo, PT_METL);
							else if (type)
								p = sim->create_part(-1, xCopyTo, yCopyTo, type);
							else
								continue;
							if (p >= 0)
							{
								if (type == PT_SPRK)
									sim->part_change_type(p, xCopyTo, yCopyTo, PT_SPRK);
								parts[p] = parts[pmap[yCurrent][xCurrent]>>8];
								parts[p].x = xCopyTo;
								parts[p].y = yCopyTo;
							}
						}
					}
				}
	}
	return 0;
}

Element_DRAY::~Element_DRAY() {}
