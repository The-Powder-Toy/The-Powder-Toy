#include "simulation/ElementCommon.h"

void Element::Element_GUNP()
{
	Identifier = "DEFAULT_PT_GUNP";
	Name = "GUN";
	Colour = PIXPACK(0xC0C0D0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.80f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 600;
	Explosive = 1;
	Meltable = 0;
	Hardness = 10;

	Weight = 85;

	HeatConduct = 97;
	Description = "Gunpowder. Light dust, explodes on contact with fire or spark.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_FIRE;
}
