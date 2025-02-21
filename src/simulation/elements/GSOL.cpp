#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_GSOL()
{
	Identifier = "DEFAULT_PT_GSOL";
	Name = "GSOL";
	Colour = 0x969530_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 2;

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 7;

	HeatConduct = 42;
	Description = "Gasoline. Slowly turns into flammable vapors. Evaporates less when cold.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 7.0f;
	HighPressureTransition = PT_FIRE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 375.0f;
	HighTemperatureTransition = PT_FIRE;

    Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp <= 375.0f ? sim->rng.chance((int)parts[i].temp, 2000000) : sim->rng.chance(375, 2000000))
	{
		sim->create_part(i, x, y, PT_GSLV);
		return -1;
	}

	return 0;
}