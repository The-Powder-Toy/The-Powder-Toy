#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LAVA PT_LAVA 6
Element_LAVA::Element_LAVA()
{
	Identifier = "DEFAULT_PT_LAVA";
	Name = "LAVA";
	Colour = PIXPACK(0xE05010);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.0003f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;
	PhotonReflectWavelengths = 0x3FF00000;

	Weight = 45;

	Temperature = R_TEMP+1500.0f+273.15f;
	HeatConduct = 60;
	Description = "Molten lava. Ignites flammable materials. Generated when metals and other materials melt, solidifies when cold.";

	Properties = TYPE_LIQUID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = MAX_TEMP;// check for lava solidification at all temperatures
	LowTemperatureTransition = ST;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FIRE::update;
	Graphics = &Element_LAVA::graphics;
}


//#TPT-Directive ElementHeader Element_LAVA static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LAVA::graphics(GRAPHICS_FUNC_ARGS)

{
	*colr = cpart->life * 2 + 0xE0;
	*colg = cpart->life * 1 + 0x50;
	*colb = cpart->life / 2 + 0x10;
	if (*colr>255) *colr = 255;
	if (*colg>192) *colg = 192;
	if (*colb>128) *colb = 128;
	*firea = 40;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	*pixel_mode |= FIRE_ADD;
	*pixel_mode |= PMODE_BLUR;
	//Returning 0 means dynamic, do not cache
	return 0;
}


Element_LAVA::~Element_LAVA() {}
