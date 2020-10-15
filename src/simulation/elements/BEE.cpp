#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_BEE()
{
	Identifier = "DEFAULT_PT_BEE";
	Name = "BEE";
	Colour = PIXPACK(0xff8000);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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

	Flammable = 1;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 91;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 42;
	Description = "BEE, Secretes wax, attacks figh/stkm, eats plant to stay alive and multiply.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 374.15f;
	HighTemperatureTransition = PT_NONE;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{

	if (RNG::Ref().chance(1, 40)) //Slowly loses life if there's nothing to eat.
	{
		parts[i].life -= 1;
	}
	if (parts[i].life >= 100)   //Life check, god sees everything.
		parts[i].life = 100;

	else if (parts[i].life <= 1)  //Everyone has to die one day.
		sim->kill_part(i);

	int r, rx, ry;
	for (rx = -4; rx < 3; rx++)
		for (ry = -4; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (parts[i].life < 90)
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.2f;  //Search areas for food if life drops below 90.
				}
				if (parts[i].life <= 30)
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.9f;  //Search wider areas for food if life drops below 30.
				}
				switch (TYP(r))
				{
				case PT_WOOD:
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = -1.0f;
					if (RNG::Ref().chance(1, 90))
					{
						if (parts[i].life > 75)
						{
							sim->create_part(-1, x + 4, y + 4, PT_WAX);
							parts[ID(r)].temp = 373.15f;
							parts[i].life--;
						}
					}
				}
				break;
				case PT_FIRE:
				case PT_PLSM:
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 3.0f;
				}
				break;
				case PT_MWAX:
				case PT_WAX:
				{
					
						parts[ID(r)].temp = 373.15f;

				}
				break;
				case PT_PLNT:
				{
					
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = -2.0f;
					parts[i].life = 100;
					if (RNG::Ref().chance(1, 90))
					{
						sim->create_part(-1, x + 4, y + 4, PT_BEE);
						sim->part_change_type(ID(r), x + rx, y + ry, PT_NONE);
					}
				}
				break;
				case PT_FIGH:
				case PT_STKM:
				case PT_STKM2:
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = -2.0f;
					if (RNG::Ref().chance(1, 30))  //Attack stkms.
						parts[ID(r)].life -= 5;
				}
				break;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	if (cpart->life <= 30)
	{
		*colr = 180;
		*colg = 180;
		*colb = 0;
	}

	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = 100;
}
