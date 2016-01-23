#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FETH PT_FETH 182
Element_FETH::Element_FETH()
{
	Identifier = "DEFAULT_PT_FETH";
	Name = "FETH";
	Colour = PIXPACK(0xF0FFFF);
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 1;

	Weight = 30;

	Temperature = R_TEMP+0.0f	+73.4f;
	HeatConduct = 251;
	Description = "Frozen Ethonol";
	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 159.3f;
	HighTemperatureTransition = PT_ETHL;

	Update = NULL;
}

Element_FETH::~Element_FETH() {}
