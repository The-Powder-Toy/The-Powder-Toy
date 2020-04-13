#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_SUN()
{
	Identifier = "DEFAULT_PT_SUN";
	Name = "SUN";
	Colour = PIXPACK(0xFFBE30);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 8000.0f + 273.15f;
	HeatConduct = 0;
	Description = "SUN.";

	Properties = TYPE_SOLID;

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
	int r, rx, ry, rt;
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (rx || ry)
			{
				if (RNG::Ref().chance(1, 90))
				{
					int r = sim->create_part(-1, x + rx, y + ry, PT_UVRD);
					if (r != -1)
					{
						parts[r].vx = rx * 3;
						parts[r].vy = ry * 3;
						if (r > i)
						{
							// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
							parts[r].flags |= FLAG_SKIPMOVE;
						}
					}
				}
			}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	{

		*firer = 100.0;
		*fireg = 8.0;
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*pixel_mode |= FIRE_ADD;
		*pixel_mode |= PMODE_FLARE;

	}
	return 0;
}