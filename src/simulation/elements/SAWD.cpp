#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SAWD()
{
	Identifier = "DEFAULT_PT_SAWD";
	Name = "SAWD";
	Colour = 0xF0F0A0_rgb;
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

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
       	Hardness = 29;

	Weight = 18;

	HeatConduct = 70;
	Description = "Sawdust. Floats on water.";

	Properties = TYPE_PART | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = nullptr; // is this needed?
}

static int update(UPDATE_FUNC_ARGS)
{
	int nearbyWax = 0;
	for (auto rx = -3; rx <= 3; rx++)
	{
		for (auto ry = -3; ry <= 3; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r) == PT_MWAX)
				{
					nearbyWax++;
				}
			}
		}
	}

	parts[i].tmp += nearbyWax;
	if (parts[i].tmp > 200)
	{
		int rx = sim->rng.between(-2, 2);
		int ry = sim->rng.between(-2, 2);
		int p = pmap[y+ry][x+rx];
		if (p && TYP(p) == PT_MWAX)
		{
			sim->create_part(i, x, y, PT_PAPR);
			sim->kill_part(ID(p));
			return 1;
		}
	}
	if (parts[i].tmp > 0)
	{
		parts[i].vx = 0;
		parts[i].vy = 0;
		parts[i].tmp--;
	}
	return 0;
}
