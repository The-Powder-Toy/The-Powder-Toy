#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PRMT()
{
	Identifier = "DEFAULT_PT_PRMT";
	Name = "PRMT";
	Colour = PIXPACK(0x008000);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 20;
	Explosive = 1;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x008000;

	Flammable = 100;
	Explosive = 1;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 35;
	Description = "Promethium, cathes fire at high velocity, emits NEUT at low temp and with PLUT, explosive at high temp.";

	Properties = TYPE_PART | PROP_NEUTPASS | PROP_RADIOACTIVE;

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
	if (parts[i].vx >= 12.0 || parts[i].vy >= 12.0)
	{
		parts[i].type = PT_FIRE;
		parts[i].life = 200;
		parts[i].temp = 574.15f;
	}
    if (parts[i].temp >= 874.15f)
	{
		parts[i].type = PT_SING;
		parts[i].tmp = 20;
	}

   if (parts[i].temp <= 274.15f)
	{
		 if (RNG::Ref().chance(1, 100))
		 {
			 sim->create_part(i, x, y, PT_NEUT);
		 }
	}

	for (int rx = -2; rx < 3; rx++)
		for (int ry = -2; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				switch (TYP(r))
				{
				case PT_PLUT:
				{
					parts[ID(r)].type = PT_NONE;
					sim->create_part(i, x, y, PT_NEUT);
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = -2.0f;
				}
				break;
				case PT_SING:
				{
					parts[i].tmp = 10;
				}
				break;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->temp <= 274.15f)
	{
		*colb = 105;
		*colr = 95;
		*colg = 65;
	}
	if (cpart->temp >= 600.15f)
	{
		*colb = 0;
		*colr = 105;
		*colg = 65;
	}
	return 0;
}
