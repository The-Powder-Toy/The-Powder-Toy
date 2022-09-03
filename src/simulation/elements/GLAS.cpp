#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_GLAS()
{
	Identifier = "DEFAULT_PT_GLAS";
	Name = "GLAS";
	Colour = PIXPACK(0x404040);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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

	HeatConduct = 150;
	Description = "Glass. Meltable. Shatters under pressure, and refracts photons.";

	Properties = TYPE_SOLID | PROP_NEUTPASS | PROP_HOT_GLOW | PROP_SPARKSETTLE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1973.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp4 >= 120) // To make life increments not so random but still way more controllable (1 for every 120 frames).
	{
		parts[i].tmp4 = 0;
		parts[i].life += 1;
	}
	if (parts[i].life < 16)// Compatibilty stuff
	{
		parts[i].life = 16;
	}
	auto press = int(sim->pv[y/CELL][x/CELL] * 64);
	auto diff = press - parts[i].tmp3;

	// Determine whether the GLAS is chemically strengthened via .life setting. (250 = Max., 16 = Min.)
		if (diff > parts[i].life || diff < -1*(parts[i].life))
		{
			sim->part_change_type(i, x, y, PT_BGLA);
		}
	parts[i].tmp3 = press;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp3 = int(sim->pv[y/CELL][x/CELL] * 64);
}
