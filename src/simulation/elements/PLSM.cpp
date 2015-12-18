#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PLSM PT_PLSM 49
Element_PLSM::Element_PLSM()
{
	Identifier = "DEFAULT_PT_PLSM";
	Name = "PLSM";
	Colour = PIXPACK(0xBB99FF);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 0.9f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.20f;
	Collision = 0.0f;
	Gravity = -0.1f;
	Diffusion = 0.30f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = 10000.0f		+273.15f;
	HeatConduct = 5;
	Description = "Plasma, extremely hot.";

	Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FIRE::update;
	Graphics = &Element_PLSM::graphics;
}

//#TPT-Directive ElementHeader Element_PLSM static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PLSM::graphics(GRAPHICS_FUNC_ARGS)

{
	int caddress = restrict_flt(restrict_flt((float)cpart->life, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)ren->plasma_data[caddress];
	*colg = (unsigned char)ren->plasma_data[caddress+1];
	*colb = (unsigned char)ren->plasma_data[caddress+2];

	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode = PMODE_GLOW | PMODE_ADD; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}


Element_PLSM::~Element_PLSM() {}
