#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_SPAWN PT_SPAWN 118
Element_SPAWN::Element_SPAWN()
{
	Identifier = "DEFAULT_PT_SPAWN";
	Name = "SPWN";
	Colour = PIXPACK(0xAAAAAA);
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
	Description = "STKM spawn point.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
	CreateAllowed = &Element_SPAWN::createAllowed;
	ChangeType = &Element_SPAWN::changeType;
}

//#TPT-Directive ElementHeader Element_SPAWN static bool createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
bool Element_SPAWN::createAllowed(ELEMENT_CREATE_ALLOWED_FUNC_ARGS)
{
	return sim->player.spawnID == -1;
}

//#TPT-Directive ElementHeader Element_SPAWN static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
void Element_SPAWN::changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (to == PT_SPAWN)
	{
		if (sim->player.spawnID == -1)
			sim->player.spawnID = i;
	}
	else
	{
		if (sim->player.spawnID == i)
			sim->player.spawnID = -1;
	}
}

Element_SPAWN::~Element_SPAWN() {}
