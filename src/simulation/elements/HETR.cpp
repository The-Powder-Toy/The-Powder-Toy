#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_HETR PT_HETR 666
Element_HETR::Element_HETR()
{
	Identifier = "DEFAULT_PT_HETR";
	Name = "HETR";
	Colour = PIXPACK(0xdb8e5e);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "Heater, takes nearby filt value starting with 0x10000000 and becomes that as a temperature (K).";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_HETR::update;
}

//#TPT-Directive ElementHeader Element_HETR static int update(UPDATE_FUNC_ARGS)
int Element_HETR::update(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	bool bl1 = false;
	int tsense = 0;
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK)
			{
				r = pmap[y + ry][x + rx];
				if (!r || (r & 0xFF) == PT_HETR)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				if ((r & 0xFF) == PT_FILT && parts[r >> 8].tmp != 2)
				{
					bl1 = true;
					tsense = parts[r >> 8].ctype - 0x10000000;
				}
				else
				{
					parts[i].temp = R_TEMP;
				}
				if (bl1) {
					parts[i].temp = tsense;
				}
			}
	return 0;
}

Element_HETR::~Element_HETR() {}
