#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NICE PT_NICE 51
Element_NICE::Element_NICE()
{
	Identifier = "DEFAULT_PT_NICE";
	Name = "NICE";
	Colour = PIXPACK(0xC0E0FF);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.0005f* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	Temperature = 35.0f;
	HeatConduct = 46;
	Description = "Nitrogen Ice. Very cold, will melt into LN2 when heated only slightly.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 63.1f;
	HighTemperatureTransition = PT_LNTG;

	Update = NULL;
}

Element_NICE::~Element_NICE() {}
