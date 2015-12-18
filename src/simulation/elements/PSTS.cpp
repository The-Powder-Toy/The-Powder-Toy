#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSTS PT_PSTS 112
Element_PSTS::Element_PSTS()
{
	Identifier = "DEFAULT_PT_PSTS";
	Name = "PSTS";
	Colour = PIXPACK(0x776677);
	MenuVisible = 0;
	MenuSection = SC_CRACKER;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Solid form of PSTE.";

	Properties = TYPE_SOLID;

	LowPressure = 0.5f;
	LowPressureTransition = PT_PSTE;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_PSTS::~Element_PSTS() {}
