#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
int Element_ROCK_update(UPDATE_FUNC_ARGS);

void Element::Element_METL()
{
	Identifier = "DEFAULT_PT_METL";
	Name = "METL";
	Colour = PIXPACK(0x404060);
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
	Description = "The basic conductor. Meltable.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = PT_LAVA;

	Graphics = &graphics;
	Update = &Element_ROCK_update;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp2 == 0)
		cpart->tmp2 = RNG::Ref().between(0, 10);
	int z = cpart->tmp2; // Randomized color noise based on tmp2 for lead only

	if (cpart->tmp == 82) // Lead Color
	{
		*colr = 80 + z;
		*colg = 80 + z;
		*colb = 80 + z;
	}
	return 0;
}
