#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MORT PT_MORT 77
Element_MORT::Element_MORT()
{
	Identifier = "DEFAULT_PT_MORT";
	Name = "MORT";
	Colour = PIXPACK(0xE0E0E0);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.01f;
	HotAir = 0.002f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	Temperature = R_TEMP+4.0f	+273.15f;
	HeatConduct = 60;
	Description = "Steam Train.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_MORT::update;
}

//#TPT-Directive ElementHeader Element_MORT static int update(UPDATE_FUNC_ARGS)
int Element_MORT::update(UPDATE_FUNC_ARGS)
{
	sim->create_part(-1, x, y-1, PT_SMKE);
	return 0;
}


Element_MORT::~Element_MORT() {}
