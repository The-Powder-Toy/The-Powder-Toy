#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PIND PT_PIND 191
Element_PIND::Element_PIND()
{
  Identifier = "DEFAULT_PT_PIND";
	Name = "PIND";
	Colour = PIXPACK(0xCCFFFF);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.1f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.10f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 10;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "General Powder. Indestructible.";

	State = ST_SOLID;
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

}

Element_PIND::~Element_PIND() {}
