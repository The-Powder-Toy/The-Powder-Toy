#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_CFLM()
{
	Identifier = "DEFAULT_PT_HFLM";
	Name = "CFLM";
	Colour = 0x8080FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.9f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.20f;
	Collision = 0.0f;
	Gravity = -0.1f;
	Diffusion = 0.00f;
	HotAir = 0.0005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 2;

	DefaultProperties.temp = 0.0f;
	HeatConduct = 88;
	Description = "Sub-zero flame.";

	Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Graphics = &graphics;
	Create = &create;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	RGB<uint8_t> color = Renderer::clfmTableAt(cpart->life / 2);
	*colr = color.Red;
	*colg = color.Green;
	*colb = color.Blue;

	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = sim->rng.between(50, 199);
}
