#include "simulation/ElementCommon.h"
static int update(UPDATE_FUNC_ARGS);
void Element::Element_CLUD()
{
	Identifier = "DEFAULT_PT_CLUD";
	Name = "CLUD";
	Colour = PIXPACK(0x303035);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.001f * CFDS;
	AirLoss = 0.9f;
	Loss = 0.002f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.1f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 91;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 22;
	Description = "Cloud, reacts with CAUS to produce Acid clouds. Randomly creates LIGH.";

	Properties = TYPE_GAS;
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
		int r, rx, ry, rndstore;
		for (int rx = -2; rx < 3; rx++)
			for (int ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y + ry][x + rx];
					if (!r)
						continue;
					if (RNG::Ref().chance(1, 200))
					{
						parts[i].tmp += 1;
					}
					
					else if (RNG::Ref().chance(6, 100000000))
					{
						sim->create_part(-1, x, y + 40, PT_LIGH);
					}
					if (parts[i].tmp >= 40 && parts[i].tmp2 !=1)
					{
						sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.2f;
						sim->create_part(-1, x, y + 30, PT_WATR);
						parts[i].tmp -= 30;
					}

					if (parts[i].tmp >= 40 && parts[i].tmp2 == 1)
					{
						sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.2f;
						sim->create_part(-1, x, y + 30, PT_ACID);
						parts[i].tmp -= 30;
					}

					if (parts[ID(r)].type == PT_CAUS)
					{
						parts[i].tmp2 = 1;
					}

					
		}
	return 0;
}



