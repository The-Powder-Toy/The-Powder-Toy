#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_FUEL PT_FUEL 195
Element_FUEL::Element_FUEL()
{
	Identifier = "DEFAULT_PT_FUEL";
	Name = "FUEL";
	Colour = PIXPACK(0x25500255);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 7;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 20;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 100;
	Description = "Flammable,burns very slowly";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = NT;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_FUEL::~Element_FUEL() {}
