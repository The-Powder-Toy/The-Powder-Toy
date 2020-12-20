#include "simulation/ElementCommon.h"

int Element_VIRS_update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_VRSG()
{
	Identifier = "DEFAULT_PT_VRSG";
	Name = "VRSG";
	Colour = PIXPACK(0xFE68FE);
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
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 500;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	DefaultProperties.temp = 522.0f + 273.15f;
	HeatConduct = 251;
	Description = "Gas Virus. Turns everything it touches into virus.";

	Properties = TYPE_GAS|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 673.0f;
	LowTemperatureTransition = PT_VIRS;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.pavg[1] = 250;

	Update = &Element_VIRS_update;
	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode &= ~PMODE;
	*pixel_mode |= FIRE_BLEND;
	*firer = *colr/2;
	*fireg = *colg/2;
	*fireb = *colb/2;
	*firea = 125;
	*pixel_mode |= NO_DECO;
	return 1;
}
