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
	Collision = -0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 70;

	Weight = 120;

	HeatConduct = 200;
	Description = "Rock. Solid material, CNCT can stack on top of it.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
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
	int r, rx, ry, rt;
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y / CELL][x / CELL];
	float diff = parts[i].pavg[1] - parts[i].pavg[0];
	if (parts[i].pavg[1] >= 50.00f && (diff > 10.00f || diff < -10.00f)) //Pressure Change Breaking Point
	{
		if (parts[i].tmp==1 && RNG::Ref().chance(1, 500)) //1 in 500 sulfides will produce GOLD
			sim->part_change_type(i, x, y, PT_GOLD);
		else if (parts[i].tmp == 1 && RNG::Ref().chance(1, 100)) // Silver 5x more likely than GOLD
		{
			parts[i].tmp = 47;
			sim->part_change_type(i, x, y, PT_GOLD);
		}
		else if (parts[i].tmp == 2 && RNG::Ref().chance(1, 20)) //1 in 20 roasted sulfides will produce GOLD
			sim->part_change_type(i, x, y, PT_GOLD);
		else if (parts[i].tmp == 2 && RNG::Ref().chance(1, 20)) // Silver
		{
			parts[i].tmp = 47;
			sim->part_change_type(i, x, y, PT_GOLD);
		}
		else
			sim->part_change_type(i, x, y, PT_STNE);
	}

	if (parts[i].tmp == 1 && (parts[i].type == PT_ROCK || parts[i].type == PT_STNE) && parts[i].temp >= 873 && RNG::Ref().chance(1, 5000)) //Allow sulfides to burn off at temperature, changing chemistry releasing CAUS and SMKE. Handles ROCK and STNE
	{
		if (RNG::Ref().chance(1, 15))
		{
			rx = RNG::Ref().chance(1, 2) - 1;
			ry = RNG::Ref().chance(1, 2) - 1;
			sim->create_part(-1, x + rx, y + ry, PT_CAUS);
		}
		rx = RNG::Ref().chance(1, 2) - 1;
		ry = RNG::Ref().chance(1, 2) - 1;
		sim->create_part(-1, x + rx, y + ry, PT_SMKE);

		if (RNG::Ref().chance(1, 500)) //1 in 500 will directly produce GOLD
			sim->part_change_type(i, x, y, PT_GOLD);
		else if (RNG::Ref().chance(1, 100)) // Silver 5x more likely than GOLD
		{
			sim->part_change_type(i, x, y, PT_GOLD);
			parts[i].tmp = 47;
		}
		else
			parts[i].tmp = 2;
	}

	if (parts[i].type == PT_ROCK && parts[i].temp >= 1943.15 && parts[i].tmp == 0) //Melting temperatures
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
	}
	else if (parts[i].type == PT_ROCK && parts[i].temp >= 1153.15 && (parts[i].tmp == 1 || parts[i].tmp == 2)) //Sulfides
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
		parts[i].tmp = 1;
	}
	else if (parts[i].type == PT_STNE && parts[i].temp >= 1153.15 && (parts[i].tmp == 1 || parts[i].tmp == 2)) //Sulfide Powders
	{
		if (parts[i].tmp == 1)
		{
			if (RNG::Ref().chance(1, 15))
			{
				rx = RNG::Ref().chance(1, 2) - 1;
				ry = RNG::Ref().chance(1, 2) - 1;
				sim->create_part(-1, x + rx, y + ry, PT_CAUS);
			}
			rx = RNG::Ref().chance(1, 2) - 1;
			ry = RNG::Ref().chance(1, 2) - 1;
			sim->create_part(-1, x + rx, y + ry, PT_SMKE);
		}

		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_STNE;
		parts[i].tmp = 2;
	}
	else if (parts[i].type == PT_STNE && parts[i].temp >= 983.0f && (parts[i].tmp != 1 && parts[i].tmp != 2))
		sim->part_change_type(i, x, y, PT_LAVA);
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp2 == 0)
		cpart->tmp2 = RNG::Ref().between(0, 10);
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
	if (cpart->tmp == 1) // Sulfide Color
	{
		*colr += 50;
		*colg += 30;
		*colb += -30;
	}
	if (cpart->tmp == 2) // Roasted Sulfide Color
	{
		*colr += 25;
		*colg += 15;
		*colb += -15;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 10);
}
