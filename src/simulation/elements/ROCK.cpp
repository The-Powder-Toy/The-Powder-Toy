#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
int Element_ROCK_update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_ROCK()
{
	Identifier = "DEFAULT_PT_ROCK";
	Name = "ROCK";
	Colour = PIXPACK(0x727272);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 200;
	Description = "Rock. Solid material, CNCT can stack on top of it.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 120;
	HighPressureTransition = PT_STNE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = ST;

	Update = &Element_ROCK_update;
	Graphics = &graphics;
	Create = &create;
}

int Element_ROCK_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp == 2 && parts[i].type == PT_ROCK && parts[i].temp >= 873 && RNG::Ref().chance(1, 5000)) //Sulfurized ROCK Heat Reactions
	{
		if (RNG::Ref().chance(1, 15))
		{
			sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_CAUS);
		}
		sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_SMKE);

		if (RNG::Ref().chance(1, 250))
			sim->part_change_type(i, x, y, PT_METL);
		else if (RNG::Ref().chance(1, 500))
			sim->part_change_type(i, x, y, PT_GOLD);
		else if (RNG::Ref().chance(1, 750)) 
			sim->part_change_type(i, x, y, PT_MERC);
	}


	/*SPECIAL HIGH TEMP TRANSITIONS*/
	if (parts[i].type == PT_ROCK && parts[i].temp >= 1943.15 && parts[i].tmp != 2) //ROCK
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
	}
	else if (parts[i].type == PT_ROCK && parts[i].temp >= 1153.15 && parts[i].tmp == 2) //Sulfurized ROCK, Lower Melting Temp
	{
		parts[i].ctype = PT_ROCK;
		sim->part_change_type(i, x, y, PT_LAVA);
	}
	return 0;
}


static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 7) * 6; // Randomized color noise based on tmp2
	*colr += z;
	*colg += z;
	*colb += z;

	if (cpart->temp >= 810.15) // Glows when hot, right before melting becomes bright
	{
		*pixel_mode |= FIRE_ADD;

		*firea = int(((cpart->temp)-810.15)/45);
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
	}
	if (cpart->tmp == 2) //Yellow Color Shift (Sulfurized ROCK Reactions)
	{
		*colr += 50;
		*colg += 30;
		*colb += -30;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 10);
}
