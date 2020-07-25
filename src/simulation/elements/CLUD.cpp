#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CLUD()
{
	Identifier = "DEFAULT_PT_CLUD";
	Name = "CLUD";
	Colour = PIXPACK(0x404039);
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
	Hardness = 1;

	Weight = 91;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 42;
	Description = "Cloud, rains when fed with WATR. Produces LIGH";

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
	Graphics = &graphics;
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
					if (RNG::Ref().chance(1, 900))
					{
						parts[i].tmp2 = 1;
					}

					if (parts[i].tmp >= 40)
					{
						sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.5f;
						sim->create_part(-1, x, y + 25, PT_WATR);
						parts[i].tmp = 0;
					}
					if (parts[i].tmp2 == 1)
					{
						sim->create_part(-1, x, y + 25, PT_LIGH);
						parts[i].tmp2 = 0;
					}
		}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 75;
	*firer = 55;
	*fireg = 55;
	*fireb = 55;

	*pixel_mode = PMODE_NONE;
	*pixel_mode |= FIRE_BLEND;

	return 1;
}

