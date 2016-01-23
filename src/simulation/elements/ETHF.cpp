#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ETHF PT_ETHF 181
Element_ETHF::Element_ETHF()
{
	Identifier = "DEFAULT_PT_ETHF";
	Name = "ETHF";
	Colour = PIXPACK(0xD0FFFF);
	MenuVisible = 0;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.75f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 600;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	Temperature = R_TEMP+2.0f	+355f;
	HeatConduct = 42;
	Description = "ethanol fumes";

	Properties = TYPE_GAS | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPL;
	HighPressureTransition = NT;
	LowTemperature = 351.4;
	LowTemperatureTransition = PT_ETHL;
	HighTemperature = 635.6f;
	HighTemperatureTransition = PT_FIRE;

	Update = NULL;
}

Element_ETHF::~Element_ETHF() {}
