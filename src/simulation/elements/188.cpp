#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E188 PT_E188 188
Element_E188::Element_E188()
{
	Identifier = "DEFAULT_PT_E188";
	Name = "E188";
	Colour = PIXPACK(0xFF60D0);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 1;

	Weight = 100;

	Temperature = 140.00f;
	HeatConduct = 251;
	Description = "Experimental element. acts like ISZS and E187.";

	Properties = TYPE_SOLID | PROP_LIFE_DEC | PROP_NOSLOWDOWN | PROP_TRANSPARENT;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 300.0f;
	HighTemperatureTransition = PT_E187;

	Update = &Element_E187::update;
	Graphics = &Element_E187::graphics;
}

Element_E188::~Element_E188() {}
