#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_UINS PT_UINS 338
Element_UINS::Element_UINS()
{
	Identifier = "DEFAULT_PT_UINS";
	Name = "UINS";
	Colour = PIXPACK(0x9EA3F1);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Unbreakable Insulator, does not conduct heat and blocks electricity.";

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

Element_UINS::~Element_UINS() {}
