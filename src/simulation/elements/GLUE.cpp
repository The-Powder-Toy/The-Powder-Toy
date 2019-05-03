#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GLUE PT_GLUE 190
Element_GLUE::Element_GLUE()
{
	Identifier = "DEFAULT_PT_GLUE";
	Name = "GLUE";
	Colour = PIXPACK(0X505050);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.6f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 7;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 20;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 72;
	Description = "GLUE,hardens when heated";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 320.0f;
	HighTemperatureTransition = PT_SGLU;

	Update = NULL;
}

Element_GLUE::~Element_GLUE() {}
