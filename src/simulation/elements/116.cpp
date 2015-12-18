#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_116 PT_116 116
Element_116::Element_116()
{
	Identifier = "DEFAULT_PT_116";
	Name = "EQVE";
	Colour = PIXPACK(0xFFE0A0);
	MenuVisible = 0;
	MenuSection = SC_CRACKER2;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 85;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "A failed shared velocity test.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_116::~Element_116() {}
