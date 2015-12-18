#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DYST PT_DYST 64
Element_DYST::Element_DYST()
{
	Identifier = "DEFAULT_PT_DYST";
	Name = "DYST";
	Colour = PIXPACK(0xBBB0A0);
	MenuVisible = 0;
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

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 80;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Dead Yeast.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 473.0f;
	HighTemperatureTransition = PT_DUST;

	Update = NULL;
}

Element_DYST::~Element_DYST() {}
