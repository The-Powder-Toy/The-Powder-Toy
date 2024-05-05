#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_OSTO()
{
	Identifier = "DEFAULT_PT_OSTO";
	Name = "OSTO";
	Colour = 0x1D2738_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.75f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 1.75f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 350.0f + 273.15f;
	HeatConduct = 42;
	Description = "Osmium Tetroxide. A poisonous compound formed from oxygen and osmium.";

	Properties = TYPE_GAS | PROP_NEUTABSORB | PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// fission, very similar to CO2 fission
	if (parts[i].temp > 9273.15 && sim->pv[y / CELL][x / CELL] > 185.0f)
	{
		if (sim->rng.chance(1, 5))
		{
			int j;
			sim->create_part(i, x, y, PT_OSIM);
			sim->create_part(-3, x, y, PT_O2);
			j = sim->create_part(-3, x, y, PT_NEUT);
			if (j != -1)
				parts[j].temp = MAX_TEMP;
			if (sim->rng.chance(1, 50))
			{
				j = sim->create_part(-3, x, y, PT_ELEC);
				if (j != -1)
					parts[j].temp = MAX_TEMP;
			}
			parts[i].temp = MAX_TEMP;
			sim->pv[y / CELL][x / CELL] += 100;
		}
	}
	return 0;
}