#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SAWD PT_SAWD 181
Element_SAWD::Element_SAWD()
{
	Identifier = "DEFAULT_PT_SAWD";
	Name = "SAWD";
	Colour = PIXPACK(0xF0F0A0);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
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

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 18;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 70;
	Description = "Sawdust. Floats on water.";

	Properties = TYPE_PART | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
	Graphics = NULL;
}

Element_SAWD::~Element_SAWD() {}
