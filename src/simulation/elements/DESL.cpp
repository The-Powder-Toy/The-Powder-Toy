#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DESL PT_DESL 58
Element_DESL::Element_DESL()
{
	Identifier = "DEFAULT_PT_DESL";
	Name = "DESL";
	Colour = PIXPACK(0x440000);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 2;

	Flammable = 2;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 15;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 42;
	Description = "Liquid diesel. Explodes under high pressure and temperatures.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 5.0f;
	HighPressureTransition = PT_FIRE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 335.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = NULL;
}

Element_DESL::~Element_DESL() {}
