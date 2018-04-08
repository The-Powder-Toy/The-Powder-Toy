#include "simulation/Elements.h"

//#TPT-Directive ElementClass Element_RAYT PT_RAYT 186
Element_RAYT::Element_RAYT() {
	Identifier = "DEFAULT_PT_RAYT";
	Name = "RAYT";
	Colour = PIXPACK(0x66ff66);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
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

	Temperature = 283.15f;
	HeatConduct = 0;
	Description = "RAYT scans in 8 directions for the element in its ctype and sparks the conductor on the opposite side";

	Properties = TYPE_SOLID | PROP_DRAWONCTYPE | PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RAYT::update;
}

#define INVERT_FILTER 0
#define IGNORE_ENERGY 1

//NOTES:
// ctype is used to store the target element, if any. (NONE is treated as a wildcard)
// tmp2 is used for settings (binary flags). The flags are as follows:
// 10: Inverts the CTYPE filter so that the element in ctype is the only thing that doesn't trigger RAYT, instead of the opposite.
// 01: Ignore energy particles

//#TPT-Directive ElementHeader Element_RAYT static int update(UPDATE_FUNC_ARGS)
int Element_RAYT::update(UPDATE_FUNC_ARGS)
{
	int rx, ry, r = 0;
	for (rx = -1; rx <= 1; rx++)
	{
		for (ry = -1; ry <= 1; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry) && x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				int rt = TYP(r);
				if ((sim->elements[rt].Properties & PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR) && parts[ID(r)].life == 0)
				{
					// Stolen from DRAY
					bool isEnergy = false;
					for (int xStep = rx*-1, yStep = ry*-1, xCurrent = x+xStep, yCurrent = y+yStep; ; xCurrent+=xStep, yCurrent+=yStep)
					{
						int rr;
						if (!(xCurrent>=0 && yCurrent>=0 && xCurrent<XRES && yCurrent<YRES))
						{
							break; // We're out of bounds! Oops!
						}
						rr = pmap[yCurrent][xCurrent];
						if (!rr)
						{
							rr = sim->photons[yCurrent][xCurrent];
							if (rr)
								isEnergy = true;
						}
						if (!rr)
							continue;

						if (parts[i].ctype == PT_NONE && TYP(rr) != PT_NONE)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
						}
						if (isEnergy)
						{
							if ((parts[i].tmp2 & 1) && ((TYP(rr) == TYP(parts[i].ctype)) xor (parts[i].tmp2 & (1 << 1))))
							{
								if (sim->parts_avg(i, ID(r), PT_INSL) != PT_INSL)
								{
									parts[ID(r)].life = 4;
									parts[ID(r)].ctype = rt;
									sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
								}
							}
							else
							{
								break;
							}
						}
						if ((TYP(rr) == TYP(parts[i].ctype)) xor (parts[i].tmp2 & (1 << 1)))
						{
							if (sim->parts_avg(i,ID(r),PT_INSL) != PT_INSL)
							{
								parts[ID(r)].life = 4;
								parts[ID(r)].ctype = rt;
								sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
							}
						}
						break;
					}
				}
				continue;
			}
		}
	}
	return 0;
}

Element_RAYT::~Element_RAYT() {}
