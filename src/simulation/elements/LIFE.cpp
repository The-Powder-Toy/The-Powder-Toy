#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LIFE()
{
	Identifier = "DEFAULT_PT_LIFE";
	Name = "LIFE";
	Colour = 0x0CAC00_rgb;
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
	auto &builtinGol = SimulationData::builtinGol;
	auto colour1 = RGB::Unpack(cpart->dcolour);
	auto colour2 = RGB::Unpack(cpart->tmp);
	if (!cpart->dcolour)
	{
		colour1 = 0xFFFFFF_rgb;
	}
	auto ruleset = cpart->ctype;
	bool renderDeco = gfctx.ren->decorationLevel != RendererSettings::decorationAntiClickbait;
	if (ruleset >= 0 && ruleset < NGOL)
	{
		if (!renderDeco || gfctx.ren->decorationLevel == RendererSettings::decorationDisabled)
		{
			colour1 = builtinGol[ruleset].colour;
			colour2 = builtinGol[ruleset].colour2;
			renderDeco = true;
		}
		ruleset = builtinGol[ruleset].ruleset;
	}
	if (renderDeco)
	{
		auto states = ((ruleset >> 17) & 0xF) + 2;
		if (states == 2)
		{
			*colr = colour1.Red;
			*colg = colour1.Green;
			*colb = colour1.Blue;
		}
		else
		{
			auto mul = (cpart->tmp2 - 1) / float(states - 2);
			*colr = int(colour1.Red   * mul + colour2.Red   * (1.f - mul));
			*colg = int(colour1.Green * mul + colour2.Green * (1.f - mul));
			*colb = int(colour1.Blue  * mul + colour2.Blue  * (1.f - mul));
		}
	}
	*pixel_mode |= NO_DECO;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &builtinGol = sd.builtinGol;
	if (v == -1)
		v = 0;
	// * 0x200000: No need to look for colours, they'll be set later anyway.
	bool skipLookup = v & 0x200000;
	v &= 0x1FFFFF;
	sim->parts[i].ctype = v;
	if (v < NGOL)
	{
		sim->parts[i].dcolour = builtinGol[v].colour.Pack();
		sim->parts[i].tmp = builtinGol[v].colour2.Pack();
		v = builtinGol[v].ruleset;
	}
	else if (!skipLookup)
	{
		auto *cgol = sd.GetCustomGOLByRule(v);
		if (cgol)
		{
			sim->parts[i].dcolour = cgol->colour1.Pack();
			sim->parts[i].tmp = cgol->colour2.Pack();
		}
	}
	sim->parts[i].tmp2 = ((v >> 17) & 0xF) + 1;
}
