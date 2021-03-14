#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
int Element_ROCK_update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_STNE()
{
	Identifier = "DEFAULT_PT_STNE";
	Name = "STNE";
	Colour = PIXPACK(0xA0A0A0);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 1;

	Weight = 90;

	HeatConduct = 150;
	Description = "Heavy particles. Meltable.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = ST;

	Graphics = &graphics;
	Update = &Element_ROCK_update;
	Create = &create;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 7) * 6; // Randomized color noise based on tmp2 for sulfides only

	if (cpart->tmp == 1) // Sulfide Color
	{
		*colr = 255;
		*colg = 208 + z + 30;
		*colb = 144 - z - 30;
	}
	if (cpart->tmp == 2) // Roasted Sulfide Color
	{
		*colr = 255 - z - 25;
		*colg = 208 - z - 15;
		*colb = 144 - z - 30;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 10);
}
