#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MTHN PT_MTHN 219
Element_MTHN::Element_MTHN()
{
	Identifier = "DEFAULT_PT_MTHN";
	Name = "MTHN";
	Colour = PIXPACK(0x82AB29);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.10f;
	Gravity = 0.00f;
	Diffusion = 3.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 4;
	Explosive = 20;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Methane. Explosive and smells very bad";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITL;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_MTHN::~Element_MTHN() {}
