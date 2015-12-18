#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SALT PT_SALT 26
Element_SALT::Element_SALT()
{
	Identifier = "DEFAULT_PT_SALT";
	Name = "SALT";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 1;

	Weight = 75;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 110;
	Description = "Salt, dissolves in water.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1173.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = NULL;
}

Element_SALT::~Element_SALT() {}
