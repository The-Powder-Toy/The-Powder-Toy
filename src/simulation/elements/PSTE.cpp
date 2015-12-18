#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSTE PT_PSTE 111
Element_PSTE::Element_PSTE()
{
	Identifier = "DEFAULT_PT_PSTE";
	Name = "PSTE";
	Colour = PIXPACK(0xAA99AA);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 31;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Colloid, Hardens under pressure.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 0.5f;
	HighPressureTransition = PT_PSTS;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 747.0f;
	HighTemperatureTransition = PT_BRCK;

	Update = NULL;
}

Element_PSTE::~Element_PSTE() {}
