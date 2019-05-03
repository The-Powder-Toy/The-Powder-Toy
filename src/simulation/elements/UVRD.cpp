#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_UVRD PT_UVRD 198
Element_UVRD::Element_UVRD()
{
	Identifier = "DEFAULT_PT_UVRD";
	Name = "UVRD";
	Colour = PIXPACK(0xDA70D6);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	Temperature = R_TEMP + 3500.0f + 273.15f;
	HeatConduct = 251;
	Description = "Invisible UV rays that kills!";

	Properties = TYPE_ENERGY | PROP_LIFE_DEC | PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
	Graphics = &Element_UVRD::graphics;
}

//#TPT-Directive ElementHeader Element_UVRD static int graphics(GRAPHICS_FUNC_ARGS)
int Element_UVRD::graphics(GRAPHICS_FUNC_ARGS)

{
	int x = 0;
	*colr = *colg = *colb = 0;
	for (x = 0; x < 12; x++) {
		*colr += (cpart->ctype >> (x + 18)) & 1;
		*colb += (cpart->ctype >> x) & 1;
	}
	for (x = 0; x < 12; x++)
		*colg += (cpart->ctype >> (x + 9)) & 1;
	x = 624 / (*colr + *colg + *colb + 1);
	*colr *= x;
	*colg *= x;
	*colb *= x;

	*firea = 100;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD | NO_DECO;
	return 1;
}


Element_UVRD::~Element_UVRD() {}
