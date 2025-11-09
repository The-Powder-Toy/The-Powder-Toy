#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_GLAS()
{
	Identifier = "DEFAULT_PT_GLAS";
	Name = "GLAS";
	Colour = 0x404040_rgb;
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

	Properties = TYPE_SOLID | PROP_NEUTPASS | PROP_PHOTPASS | PROP_HOT_GLOW | PROP_SPARKSETTLE;

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
	auto press = int(sim->pv[y/CELL][x/CELL] * 64);
	auto diff = press - parts[i].tmp3;

	// Determine whether the GLAS is chemically strengthened via .life setting. (250 = Max., 16 = Min.)
	int strength = (parts[i].life / 120) + 16;
	if (strength < 16)
		strength = 16;
	if (diff > strength || diff < -1 * strength)
	{
		sim->part_change_type(i, x, y, PT_BGLA);
	}
	parts[i].tmp3 = press;

	// Liquid nitrogen condenses on chemically strengthened glass
	if ((strength > 200) && (parts[i].temp < 77.0f) && (sim->hv[y/CELL][x/CELL] < 77.0f)
			&& (sim->pv[y/CELL][x/CELL] > 5.0f) && sim->rng.chance(1, 100))
	{
		// Sample 4 adjacent cells
		auto adj = sim->rng.between(0, 3);
		auto rx = (1 - 2*(adj%2))*(1 - adj/2);
		auto ry = (1 - 2*(adj%2))*(adj/2);

		auto r = pmap[y+ry][x+rx];
		// If found an empty spot around glass
		if (!r)
		{
			auto np = sim->create_part(-1, x+rx, y+ry, PT_LNTG);
			if (np>-1)
			{
				sim->pv[y/CELL][x/CELL] -= 1.0f;
				parts[i].temp += 200.0f;
			}
		}
	}

	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp3 = int(sim->pv[y/CELL][x/CELL] * 64);
}
