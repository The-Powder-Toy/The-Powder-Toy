#include "simulation/ElementCommon.h"

int Element_FNTC_update(UPDATE_FUNC_ARGS);

void Element::Element_FNTC()
{
	Identifier = "DEFAULT_PT_FNTC";
	Name = "FNTC";
	Colour = PIXPACK(0x505040);
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
	Meltable = 1;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "Faster NTCT.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1687.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_FNTC_update;
}

int Element_FNTC_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 295.0f)
		parts[i].temp -= 7.5f;
	return 0;
}
