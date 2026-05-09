#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS);

void Element::Element_SPAWN2()
{
	Identifier = "DEFAULT_PT_SPAWN2";
	Name = "SPWN2";
	Colour = 0xAAAAAA_rgb;
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "STK2 spawn point.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	CreateAllowed = &createAllowed;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (sim->elementCount[PT_STKM2] <= 0 && !sim->player2.spwn)
	{
		auto si = sim->create_part(-1, x, y, PT_STKM2);
		if (si >= 0) // Make old SPAWN create old stickmen
			parts[si].flags |= parts[i].flags & FLAG_OLDSTKM;
	}

	return 0;
}

static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return sim->elementCount[PT_SPAWN2] <= 0;
}
