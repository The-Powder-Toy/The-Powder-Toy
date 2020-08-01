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
	Description = "BEE, Eats PLNT and secretes wax, Attacks figh/stkm. Eats wood to stay alive.";

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
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{

	if (RNG::Ref().chance(1, 50)) //Slowly loses life if there's nothing to eat.
	{
		parts[i].life -= 1;
	}
	if (parts[i].life >= 100)   //Life check, god sees everything.
		parts[i].life = 100;

	else if (parts[i].life <= 1)  //Everyone has to die one day.
		parts[i].type = PT_NONE;

	int r, rx, ry;
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (parts[i].life <= 30)
				{
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] = 0.7f;  //Search wider areas for food if life drops below 30.
				}
				switch (TYP(r))
				{
				case PT_PLNT:
				{
					if (RNG::Ref().chance(1, 90))
					{
						sim->part_change_type(ID(r), x + rx, y + ry, PT_MWAX);
						parts[ID(r)].temp = 323.15f;
					}

				}
				break;
				case PT_WOOD:
				{
					parts[i].life += 50;
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
	if (cpart->life <= 30)
	{
		*colr = 180;
		*colg = 120;
		*colb = 0;
	}
	return 0;
}



static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = 100;
}
