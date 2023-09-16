#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FUSE()
{
	Identifier = "DEFAULT_PT_FUSE";
	Name = "FUSE";
	Colour = 0x0A5706_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	HeatConduct = 200;
	Description = "Burns slowly. Ignites at somewhat high temperatures or with electricity.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 50;
	DefaultProperties.tmp = 50;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life<=0) {
		auto r = sim->create_part(i, x, y, PT_PLSM);
		if (r>-1)
			parts[r].life = 50;
		return 1;
	}
	else if (parts[i].life < 40) {
		parts[i].life--;
		if (sim->rng.chance(1, 100)) {
			auto r = sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_PLSM);
			if (r>-1)
				parts[r].life = 50;
		}
	}
	if ((sim->pv[y/CELL][x/CELL] > 2.7f) && parts[i].tmp>40)
		parts[i].tmp=39;
	else if (parts[i].tmp<=0) {
		sim->create_part(i, x, y, PT_FSEP);
		return 1;
	}
	else if (parts[i].tmp<40)
		parts[i].tmp--;

	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SPRK || (parts[i].temp>=(273.15+700.0f) && sim->rng.chance(1, 20)))
				{
					if (parts[i].life > 40)
						parts[i].life = 39;
				}
			}
		}
	}
	return 0;
}
