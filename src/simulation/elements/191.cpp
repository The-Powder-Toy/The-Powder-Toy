#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E191 PT_E191 191
Element_E191::Element_E191()
{
	Identifier = "DEFAULT_PT_E191";
	Name = "E191";
	Colour = PIXPACK(0xE0E0A0);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
#if (defined(DEBUG) || defined(SNAPSHOT)) && MOD_ID == 0
	Enabled = 1;
#else
	Enabled = 0;
#endif

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
	Description = "Experimental element.";

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
	Graphics = NULL;
}

Element_E191::~Element_E191() {}
