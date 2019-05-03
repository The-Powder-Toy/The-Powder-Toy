#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_COND PT_COND 196
Element_COND::Element_COND()
{
	Identifier = "DEFAULT_PT_COND";
	Name = "COND";
	Colour = PIXPACK(0x005000);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 0;
	Description = "Conditioner,set the TEMP of near by particles based on its TEMP.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_COND::update;
}
//#TPT-Directive ElementHeader Element_COND static int update(UPDATE_FUNC_ARGS)
int Element_COND::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np, rndstore;
	for (rx = -3; rx < 3; rx++)
		for (ry = -3; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (r)
				{
					parts[ID(r)].temp = parts[i].temp;
				}
				break;
				
			}
	return 0;
}
Element_COND::~Element_COND() {}
