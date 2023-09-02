#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FSEP()
{
	Identifier = "DEFAULT_PT_FSEP";
	Name = "FSEP";
	Colour = 0x63AD5F_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 70;

	HeatConduct = 70;
	Description = "Fuse Powder. Burns slowly like FUSE.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 50;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life<=0) {
		auto r = sim->create_part(i, x, y, PT_PLSM);
		if (r!=-1)
			parts[r].life = 50;
		return 1;
	}
	else if (parts[i].life < 40) {
		parts[i].life--;
		if (sim->rng.chance(1, 10)) {
			auto r = sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_PLSM);
			if (r>-1)
				parts[r].life = 50;
		}
	}
	else {
		for (auto rx = -2; rx <= 2; rx++)
		{
			for (auto ry = -2; ry <= 2; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_SPRK || (parts[i].temp>=(273.15+400.0f))) && parts[i].life>40 && sim->rng.chance(1, 15))
					{
						parts[i].life = 39;
					}
				}
			}
		}
	}
	return 0;
}
