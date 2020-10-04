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
	auto colour1 = cpart->dcolour;
	auto colour2 = cpart->tmp;
	if (!colour1)
	{
		colour1 = PIXPACK(0xFFFFFF);
	}
	auto ruleset = cpart->ctype;
	if (ruleset >= 0 && ruleset < NGOL)
	{
		ruleset = builtinGol[ruleset].ruleset;
	}
	if (ren->decorations_enable)
	{
		auto states = (ruleset >> 17) + 2;
		if (states == 2)
		{
			*colr = PIXR(colour1);
			*colg = PIXG(colour1);
			*colb = PIXB(colour1);
		}
		else
		{
			auto mul = (cpart->tmp2 - 1) / float(states - 2);
			*colr = PIXR(colour1) * mul + PIXR(colour2) * (1.f - mul);
			*colg = PIXG(colour1) * mul + PIXG(colour2) * (1.f - mul);
			*colb = PIXB(colour1) * mul + PIXB(colour2) * (1.f - mul);
		}
	}
	*pixel_mode |= NO_DECO;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].ctype = v;
	if (v < NGOL)
	{
		sim->parts[i].dcolour = builtinGol[v].colour;
		sim->parts[i].tmp = builtinGol[v].colour2;
		v = builtinGol[v].ruleset;
	}
	sim->parts[i].tmp2 = (v >> 17) + 1;
}
