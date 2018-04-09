#include "simulation/Elements.h"
#include <iostream>

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

const int mask_invert_filter = 1<<0; // First flag
const int mask_ignore_energy = 1<<1;
const int mask_no_copy_color = 1<<2;

//NOTES:
// ctype is used to store the target element, if any. (NONE is treated as a wildcard)
// tmp2 is used for settings (binary flags). The flags are as follows:
// 1000: Inverts the CTYPE filter so that the element in ctype is the only thing that doesn't trigger RAYT, instead of the opposite.
// 0100: Ignore energy particles
// 0010: Ignore FILT

bool accepted_type(Simulation* sim, int part) {
	int rt = TYP(part);
	if ((sim->elements[rt].Properties & PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR)) {
		return true;
	} else if (rt == PT_FILT || rt == PT_PHOT || rt == PT_BRAY) {
		return true;
	}
	return false;
}

bool phot_data_type(int rt) {
	if (rt == PT_FILT || rt == PT_PHOT || rt == PT_BRAY) {
		return true;
	}
	return false;
}


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

				if (accepted_type(sim, r) && parts[ID(r)].life == 0)
				{
					// Stolen from DRAY
					int xStep = rx*-1, yStep = ry*-1;
					int xCurrent = x+(xStep*(parts[i].life+1)), yCurrent = y+(yStep*(parts[i].life+1));
					for (;(parts[i].tmp == 0) || !(xCurrent-x >= parts[i].tmp) || (yCurrent-y >= parts[i].tmp);xCurrent+=xStep, yCurrent+=yStep)
					{
						int rr = pmap[yCurrent][xCurrent];
						if (!(xCurrent>=0 && yCurrent>=0 && xCurrent<XRES && yCurrent<YRES))
						{
							break; // We're out of bounds! Oops!
						}
						if (!rr)
						{
							rr = sim->photons[yCurrent][xCurrent];
							if (!(rr && !(parts[i].tmp2 & mask_ignore_energy))) {
								continue;
							}
						}
						if (!rr)
							continue;

						if (!phot_data_type(TYP(r)))
						{
							if (parts[i].ctype == 0 || (parts[i].ctype == TYP(rr)) ^ (parts[i].tmp2 & mask_invert_filter))
							{
								parts[ID(r)].life = 4;
								parts[ID(r)].ctype = TYP(r);
								sim->part_change_type(ID(r), xCurrent, yCurrent, PT_SPRK);
								break;
							}
							// room for more conditions here.
						} else {
							if (parts[i].ctype == 0 || (parts[i].ctype == TYP(rr)) ^ (parts[i].tmp2 & mask_invert_filter))
							{
								if ((phot_data_type(TYP(rr)) && !(parts[i].tmp2 & mask_no_copy_color)))
								{
									parts[ID(r)].ctype = Element_FILT::getWavelengths(&parts[ID(rr)]);
									parts[ID(r)].tmp = 8008; // verifying it works.
									break;
								}
							}
						}
					}
				}
				continue;
			}
		}
	}
	return 0;
}

Element_RAYT::~Element_RAYT() {}
