#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LIFE()
{
	Identifier = "DEFAULT_PT_LIFE";
	Name = "LIFE";
	Colour = PIXPACK(0x0CAC00);
	MenuVisible = 0;
	MenuSection = SC_LIFE;
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
	Hardness = 0;

	Weight = 100;

	DefaultProperties.temp = 9000.0f;
	HeatConduct = 40;
	Description = "Game Of Life! B3/S23";

	Properties = TYPE_SOLID|PROP_LIFE;

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
	pixel pc = 0;
	if (cpart->ctype==NGT_LOTE)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(255, 128, 0);
		else if (cpart->tmp==1)
			pc = PIXRGB(255, 255, 0);
		else
			pc = PIXRGB(255, 0, 0);
	}
	else if (cpart->ctype==NGT_FRG2)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(0, 100, 50);
		else
			pc = PIXRGB(0, 255, 90);
	}
	else if (cpart->ctype==NGT_STAR)//colors for life states
	{
		if (cpart->tmp==4)
			pc = PIXRGB(0, 0, 128);
		else if (cpart->tmp==3)
			pc = PIXRGB(0, 0, 150);
		else if (cpart->tmp==2)
			pc = PIXRGB(0, 0, 190);
		else if (cpart->tmp==1)
			pc = PIXRGB(0, 0, 230);
		else
			pc = PIXRGB(0, 0, 70);
	}
	else if (cpart->ctype==NGT_FROG)//colors for life states
	{
		if (cpart->tmp==2)
			pc = PIXRGB(0, 100, 0);
		else
			pc = PIXRGB(0, 255, 0);
	}
	else if (cpart->ctype==NGT_BRAN)//colors for life states
	{
		if (cpart->tmp==1)
			pc = PIXRGB(150, 150, 0);
		else
			pc = PIXRGB(255, 255, 0);
	}
	else if (cpart->ctype >= 0 && cpart->ctype < NGOL)
	{
		pc = builtinGol[cpart->ctype].colour;
	}
	if (pc)
	{
		*colr = PIXR(pc);
		*colg = PIXG(pc);
		*colb = PIXB(pc);
	}
	else
	{
		auto colour = cpart->dcolour;
		if (!colour)
		{
			colour = PIXPACK(0xFF00FF);
		}
		auto mul = cpart->tmp / float((cpart->ctype >> 17) + 1);
		*colr = PIXR(colour) * mul;
		*colg = PIXG(colour) * mul;
		*colb = PIXB(colour) * mul;
	}
	*pixel_mode |= NO_DECO;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].ctype = v;
	auto ruleset = (unsigned int)v;
	if (ruleset < NGOL)
	{
		ruleset = builtinGol[ruleset].ruleset;
	}
	sim->parts[i].tmp = (ruleset >> 17) + 1;
}
