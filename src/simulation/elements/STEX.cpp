#include "simulation/ElementCommon.h"

void Element::Element_STEX()
{
	Identifier = "DEFAULT_PT_STEX";
	Name = "STEX";
	Colour = PIXPACK(0xD7C0D0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.0f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 3500;
	Explosive = 1523;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	HeatConduct = 234;
	Description = "Strong Explosive, as it's name suggest, it is a strong explosive.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 248.7f;
	HighPressureTransition = PT_STEP;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3532.72f;
	HighTemperatureTransition = PT_PLSM;
}
