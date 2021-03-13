#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static int update(UPDATE_FUNC_ARGS);

void Element::Element_ROCK()
{
	Identifier = "DEFAULT_PT_ROCK";
	Name = "ROCK";
	Colour = PIXPACK(0x727272);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.00f;
	Collision = -0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 70;

	Weight = 120;

	HeatConduct = 200;
	Description = "Rock. Solid material, CNCT can stack on top of it.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 120;
	HighPressureTransition = PT_STNE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1943.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
}


static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp2 <= 0) // Randomly set tmp2 if leq 0
	{
		parts[i].tmp2 = (rand() % 5);
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2) * 7; // Randomized color noise based on tmp2
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}
