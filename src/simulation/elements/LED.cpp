#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
void Element::Element_LED()
{
	Identifier = "DEFAULT_PT_LED";
	Name = "LED";
	Colour = PIXPACK(0x404040);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
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
	HeatConduct = 0;
	Description = "Light emitting diode, .tmp2 changes colours. Temp sets brightness.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = NT;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{

	{
		int r, rx, ry, np;
		if (parts[i].life != 10)
		{
			if (parts[i].life > 0)
				parts[i].life--;
		}
		else
		{
			for (rx = -2; rx < 3; rx++)
				for (ry = -2; ry < 3; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y + ry][x + rx];
						if (!r)
							continue;
						if (TYP(r) == PT_LED)
						{
							if (parts[ID(r)].life < 10 && parts[ID(r)].life>0)
								parts[i].life = 9;
							else if (parts[ID(r)].life == 0)
								parts[ID(r)].life = 10;
						}
					}
		}

		{
			if (parts[i].temp > 374.15f)
				parts[i].temp = 374.15f;
			if (parts[i].temp < 274.15f)
				parts[i].temp = 274.15f;
		}
		return 0;
	}
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	{
		if (cpart->life == 0)
		{
			if (cpart->tmp2 == 1)                            // Different tmp modes change colour of glow.
			{
				*firer = 250;
				*fireg = 0;
				*fireb = 0;
			}
			if (cpart->tmp2 == 2)
			{
				*firer = 0;
				*fireg = 250;
				*fireb = 0;
			}
			if (cpart->tmp2 == 3)
			{
				*firer = 0;
				*fireg = 0;
				*fireb = 250;
			}

			if (cpart->tmp2 == 4)
			{
				*firer = 250;
				*fireg = 250;
				*fireb = 0;
			}

			if (cpart->tmp2 == 5)
			{
				*firer = 250;
				*fireg = 0;
				*fireb = 250;
			}

			if (cpart->tmp2 == 0 || cpart->tmp2 > 5)
			{
				*firer = 250;
				*fireg = 250;
				*fireb = 250;
				*firea = 35;
			}
			*firea = cpart->temp - 273.15f;
			*colr += *firer;
			*colg += *fireg;
			*colb += *fireb;
			*pixel_mode |= FIRE_ADD;
		}
		return 0;
	}
}
