#include "simulation/ElementCommon.h"

void Element::Element_GSLV()
{
	Identifier = "DEFAULT_PT_GSLV";
	Name = "GSLV";
	Colour = 0x62631E_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 1.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 1;

	Flammable = 1000;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	HeatConduct = 42;
	Description = "Gasoline vapors. Highly flammable.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 10.0f;
	HighPressureTransition = PT_FIRE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 375.0f;
	HighTemperatureTransition = PT_FIRE;
}