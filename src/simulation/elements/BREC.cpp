#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BREC()
{
	Identifier = "DEFAULT_PT_BREC";
	Name = "BREL";
	Colour = PIXPACK(0x707060);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.18f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 90;

	HeatConduct = 211;
	Description = "Broken electronics. Formed from EMP blasts, and when constantly sparked while under pressure, turns to EXOT.";

	Properties = TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

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
	if (parts[i].life)
	{
		if (sim->pv[y/CELL][x/CELL]>10.0f)
		{
			if (parts[i].temp>9000 && sim->pv[y/CELL][x/CELL]>30.0f && RNG::Ref().chance(1, 200))
			{
				sim->part_change_type(i, x ,y ,PT_EXOT);
				parts[i].life = 1000;
			}
			parts[i].temp += (sim->pv[y/CELL][x/CELL])/8;
		}

	}
	return 0;
}
