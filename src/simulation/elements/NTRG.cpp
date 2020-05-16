#include "simulation/ElementCommon.h"
static int update(UPDATE_FUNC_ARGS);

void Element::Element_NTRG()
{
	Identifier = "DEFAULT_PT_NTRG";
	Name = "NTRG";
	Colour = PIXPACK(0x101010);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.75f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 42;
	Description = "Nitrogen, colourless and odourless gas. Liquefies into LN2 when cooled below 190C or when under pressure.";

	Properties = TYPE_GAS| PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 12.0f;
	HighPressureTransition = PT_LNTG;
	LowTemperature = 174.15f;
	LowTemperatureTransition = PT_LNTG;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}
static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np, rndstore;
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_H2:
				{
					parts[i].type = PT_NITR;
					parts[ID(r)].type = PT_NONE;

				}
				break;
				case PT_FIRE:
				case PT_PLSM:
				{
					parts[ID(r)].type = PT_NONE;
				}
				break;
				default:
				continue;
				}

			}
	return 0;
}