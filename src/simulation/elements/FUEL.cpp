#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
void Element::Element_FUEL()
{

	Identifier = "DEFAULT_PT_FUEL";
	Name = "FUEL";
	Colour = PIXPACK(0xDB2020);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.0001f	* CFDS;
	Falldown = 2;

	Flammable = 100;
	Explosive = 1;
	Meltable = 0;
	Hardness = 5;

	Weight = 20;
	HeatConduct = 255;
	Description = "Extremely powerful fuel.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 14.0;
	HighPressureTransition = PT_PLSM;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{

	for (int rx = -2; rx < 3; rx++)
		for (int ry = -2; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				if (parts[i].temp > R_TEMP + 50.0f + 273.15f)
				{
					parts[i].temp = 1990.15f;
					parts[i].life = 130;
					parts[i].type = PT_PLSM;
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 10.0f;
				}
			}
	return 0;
}
