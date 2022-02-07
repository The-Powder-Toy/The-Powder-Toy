#include "simulation/ElementCommon.h"

void Element::Element_BHGL()
{
	Identifier = "DEFAULT_PT_BHGL";
	Name = "BHGL";
	Colour = PIXPACK(0x6FC3B7);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 510;
	Description = "Heat Glow, as it's name suggests, it glows when hot and it is breakable";

	Properties = TYPE_SOLID||PROP_HOT_GLOW|;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 55.6f;
	HighPressureTransition = PT_NONE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 9999.0f;
	HighTemperatureTransition = NT;
}
