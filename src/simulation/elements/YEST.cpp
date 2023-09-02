#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_YEST()
{
	Identifier = "DEFAULT_PT_YEST";
	Name = "YEST";
	Colour = 0xEEE0C0_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 15;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 80;

	HeatConduct = 70;
	Description = "Yeast, grows when warm (~37C).";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 373.0f;
	HighTemperatureTransition = PT_DYST;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_DYST && sim->rng.chance(1, 6) && !sim->legacy_enable)
				{
					sim->part_change_type(i,x,y,PT_DYST);
				}
			}
		}
	}
	if (parts[i].temp > 303 && parts[i].temp < 317) {
		sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_YEST);
	}
	return 0;
}
