#include "simulation/ElementCommon.h"

int Element_BIZR_update(UPDATE_FUNC_ARGS);
int Element_BIZR_graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BIZRS()
{
	Identifier = "DEFAULT_PT_BIZRS";
	Name = "BIZS";
	Colour = PIXPACK(0x00E455);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
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
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 300.0f + 273.15f;
	HeatConduct = 251;
	Description = "Bizarre solid.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 400.0f;
	LowTemperatureTransition = PT_BIZR;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.ctype = 0x47FFFF;

	Update = &Element_BIZR_update;
	Graphics = &Element_BIZR_graphics;
}
