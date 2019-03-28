#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_INSD PT_INSD 193
Element_INSD::Element_INSD()
{
	Identifier = "DEFAULT_PT_INSD";
	Name = "INSD";
	Colour = PIXPACK(0x1EA2B3);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 0;
	Description = "Insulator, does not conduct heat and blocks electricity and now indestructable!.";

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
}

Element_INSD::~Element_INSD() {}
