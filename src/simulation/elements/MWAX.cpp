#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MWAX PT_MWAX 34
Element_MWAX::Element_MWAX()
{
	Identifier = "DEFAULT_PT_MWAX";
	Name = "MWAX";
	Colour = PIXPACK(0xE0E0AA);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000001f* CFDS;
	Falldown = 2;

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 25;

	Temperature = R_TEMP+28.0f+273.15f;
	HeatConduct = 44;
	Description = "Liquid Wax. Hardens into WAX at 45 degrees.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 318.0f;
	LowTemperatureTransition = PT_WAX;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = NULL;
}

Element_MWAX::~Element_MWAX() {}
