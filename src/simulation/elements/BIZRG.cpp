#include "simulation/ElementCommon.h"
#include "BIZR.h"

void Element::Element_BIZRG()
{
	Identifier = "DEFAULT_PT_BIZRG";
	Name = "BIZG";
	Colour = 0x00FFBB_rgb;
	MenuVisible = 0;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 2.75f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP - 200.0f + 273.15f;
	HeatConduct = 42;
	Description = "Bizarre gas.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 100.0f;
	HighTemperatureTransition = PT_BIZR;

	DefaultProperties.ctype = 0x47FFFF;

	Update = &Element_BIZR_update;
	Graphics = &Element_BIZR_graphics;
}
