#include "simulation/Elements.h"
#include <iostream>

//#TPT-Directive ElementClass Element_LDTC PT_LDTC 186
Element_LDTC::Element_LDTC()
{
	Identifier = "DEFAULT_PT_LDTC";
	Name = "LDTC";
	Colour = PIXPACK(0x66ff66);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 0;
	Description = "Linear detector. Scans in 8 directions for particles with its ctype and creates a spark on the opposite side.";

	Properties = TYPE_SOLID | PROP_DRAWONCTYPE | PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_LDTC::update;
}

//#TPT-Directive ElementHeader Element_LDTC static const int FLAG_INVERT_FILTER
//#TPT-Directive ElementHeader Element_LDTC static const int FLAG_IGNORE_ENERGY
//#TPT-Directive ElementHeader Element_LDTC static const int FLAG_NO_COPY_COLOR
//#TPT-Directive ElementHeader Element_LDTC static const int FLAG_KEEP_SEARCHING
const int Element_LDTC::FLAG_INVERT_FILTER =  0x1;
const int Element_LDTC::FLAG_IGNORE_ENERGY =  0x2;
const int Element_LDTC::FLAG_NO_COPY_COLOR =  0x4;
const int Element_LDTC::FLAG_KEEP_SEARCHING = 0x8;

//NOTES:
// ctype is used to store the target element, if any. (NONE is treated as a wildcard)
// life is used for the amount of pixels to skip before starting the scan. Starts just in front of the LDTC if 0.
// tmp is the number of particles that will be scanned before scanning stops. Unbounded if 0.
// tmp2 is used for settings (binary flags). The flags are as follows:
// 0x01: Inverts the CTYPE filter so that the element in ctype is the only thing that doesn't trigger LDTC, instead of the opposite.
// 0x02: Ignore energy particles
// 0x04: Ignore FILT (do not use color copying mode)
// 0x08: Keep searching even after finding a particle


//#TPT-Directive ElementHeader Element_LDTC static bool phot_data_type(int rt);
/* Returns true for particles that activate the special FILT color copying mode */
bool Element_LDTC::phot_data_type(int rt)
{
	return rt == PT_FILT || rt == PT_PHOT || rt == PT_BRAY;
}

//#TPT-Directive ElementHeader Element_LDTC static bool accepted_conductor(Simulation *sim, int rt);
/* Returns true for particles that start a ray search ("dtec" mode)
 */
bool Element_LDTC::accepted_conductor(Simulation* sim, int r)
{
	int rt = TYP(r);
	return (sim->elements[rt].Properties & PROP_CONDUCTS) &&
		!(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT ||
		rt == PT_PTCT || rt == PT_INWR) &&
		sim->parts[ID(r)].life == 0;
}

//#TPT-Directive ElementHeader Element_LDTC static int update(UPDATE_FUNC_ARGS)
int Element_LDTC::update(UPDATE_FUNC_ARGS)
{
	int ctype = TYP(parts[i].ctype), ctypeExtra = ID(parts[i].ctype), detectLength = parts[i].tmp, detectSpaces = parts[i].tmp2;
	bool copyColor = !(parts[i].tmp2 & Element_LDTC::FLAG_NO_COPY_COLOR);
	bool ignoreEnergy = parts[i].tmp2 & Element_LDTC::FLAG_IGNORE_ENERGY;
	bool invertFilter = parts[i].tmp2 & Element_LDTC::FLAG_INVERT_FILTER;
	bool keepSearching = parts[i].tmp2 & Element_LDTC::FLAG_KEEP_SEARCHING;
	if (detectSpaces < 0)
		detectSpaces = parts[i].tmp2 = 0;
	if (detectLength < 0)
		detectLength = parts[i].tmp = 0;
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				bool boolMode = Element_LDTC::accepted_conductor(sim, r);
				bool filtMode = copyColor && TYP(r) == PT_FILT;
				if (!boolMode && !filtMode)
					continue;

				int maxRange = parts[i].life + parts[i].tmp;
				int xStep = rx * -1, yStep = ry * -1;
				int xCurrent = x + (xStep * (parts[i].life + 1)), yCurrent = y + (yStep * (parts[i].life + 1));
				for (; !parts[i].tmp ||
					(xStep * (xCurrent - x) <= maxRange &&
					yStep * (yCurrent - y) <= maxRange);
					xCurrent += xStep, yCurrent += yStep)
				{
					if (!(xCurrent>=0 && yCurrent>=0 && xCurrent<XRES && yCurrent<YRES))
						break; // We're out of bounds! Oops!
					int rr = pmap[yCurrent][xCurrent];
					if (!rr && !ignoreEnergy)
						rr = sim->photons[yCurrent][xCurrent];
					if (!rr)
						continue;

					// If ctype isn't set (no type restriction), or ctype matches what we found
					// Can use .tmp2 flag to invert this
					bool matchesCtype = parts[i].ctype == TYP(rr) && (ctype != PT_LIFE || parts[ID(rr)].ctype == ctypeExtra);
					bool matchesFilter = !ctype || (invertFilter ^ (int)matchesCtype);
					if (!matchesFilter)
					{
						if (keepSearching)
							continue;
						else
							break;
					}
					// room for more conditions here.

					if (boolMode)
					{
						parts[ID(r)].life = 4;
						parts[ID(r)].ctype = TYP(r);
						sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
						break;
					}

					if (filtMode)
					{
						if (!Element_LDTC::phot_data_type(TYP(rr)))
							continue;

						int nx = x + rx, ny = y + ry;
						int photonWl = TYP(rr) == PT_FILT ?
							Element_FILT::getWavelengths(&parts[ID(rr)]) :
							parts[ID(rr)].ctype;
						while (TYP(r) == PT_FILT)
						{
							parts[ID(r)].ctype = photonWl;
							nx += rx;
							ny += ry;
							if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
								break;
							r = pmap[ny][nx];
						}
						break;
					}
				}
			}
		}
	}
	return 0;
}

Element_LDTC::~Element_LDTC() {}
