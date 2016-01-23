#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ETHL PT_ETHL 180
Element_ETHL::Element_ETHL()
{
	Identifier = "DEFAULT_PT_ETHL";
	Name = "ETHL";
	Colour = PIXPACK(0xD0FFFF);
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 30;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 42;
	Description = "ethonol. Flammable,";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = ITL;
	HighPressureTransition = NT;
	LowTemperature = 159.321;
	LowTemperatureTransition = PT_FETH;
	HighTemperature = 351.4f;
	HighTemperatureTransition = PT_ETHF;
	Update = NULL;
}

Element_ETHL::~Element_ETHL() {}
