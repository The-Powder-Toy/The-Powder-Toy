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
	Hardness = 70;

	Weight = 100;

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
	int rx, ry;
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y / CELL][x / CELL];
	float diff = parts[i].pavg[1] - parts[i].pavg[0];
	if (parts[i].type == PT_ROCK && parts[i].pavg[1] >= 50.00f && (diff > 10.00f || diff < -10.00f)) //Break ROCK when pressure differential is greater than +/- 10
	{
		if (parts[i].tmp == 1) //Sulfides
		{
			if (RNG::Ref().chance(1, 500))
				sim->part_change_type(i, x, y, PT_GOLD);
			else if (RNG::Ref().chance(1, 100))
			{
				parts[i].tmp = 47; //Silver
				sim->part_change_type(i, x, y, PT_GOLD);
			}
		}
		else if (parts[i].tmp == 2) //Roasted Sulfides
		{
			if (RNG::Ref().chance(1, 20))
				sim->part_change_type(i, x, y, PT_GOLD);
			else if (RNG::Ref().chance(1, 20))
			{
				parts[i].tmp = 47; //Silver
				sim->part_change_type(i, x, y, PT_GOLD);
			}
		}
		else if (parts[i].tmp == 3) //Galena
		{
			if (RNG::Ref().chance(1, 20))
			{
				parts[i].tmp = 82; //Lead
				sim->part_change_type(i, x, y, PT_METL);
			}
			else if (RNG::Ref().chance(1, 60))
			{
				parts[i].tmp = 47; //Silver
				sim->part_change_type(i, x, y, PT_GOLD);
			}
		}
		if (parts[i].type == PT_ROCK) //Only break if current part is still ROCK
			sim->part_change_type(i, x, y, PT_STNE);
	}

	if (parts[i].tmp == 1 && (parts[i].type == PT_ROCK || parts[i].type == PT_STNE) && parts[i].temp >= 873 && RNG::Ref().chance(1, 5000)) //Sulfide "Roasting" reactions
	{
		if (RNG::Ref().chance(1, 15))
		{
			//This is my lazy way of doing it. It has a good chance of simply not creating a particle because one already exists in the random location. I got rid of setting the rand to variables because they are only used right here
			//Although it's lazy it's much more efficient than checking all surrounding spaces. Delete this comment and the one above after review. 
			sim->create_part(-1, x + RNG::Ref().chance(1, 3) -2, y + RNG::Ref().chance(1, 3) -2, PT_CAUS);
		}
		sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_SMKE);

		if (RNG::Ref().chance(1, 500))
			sim->part_change_type(i, x, y, PT_GOLD);
		else if (RNG::Ref().chance(1, 100))
		{
			sim->part_change_type(i, x, y, PT_GOLD);
			parts[i].tmp = 47; //Silver
		}
		else if (RNG::Ref().chance(1, 500)) //Maybe add Cinnabar (Mercury Sulfide) later
			sim->part_change_type(i, x, y, PT_MERC);
		else
			parts[i].tmp = 2; //Set to roasted Sulfide
	}
	
	/*SPECIAL HIGH TEMP TRANSITIONS*/
	if (parts[i].type == PT_ROCK && parts[i].temp >= 1943.15 && parts[i].tmp == 0) //Normal ROCK
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
	}
	else if (parts[i].type == PT_ROCK && parts[i].temp >= 1153.15 && (parts[i].tmp == 1 || parts[i].tmp == 2)) //Sulfides
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
	}
	else if (parts[i].type == PT_STNE && parts[i].temp >= 1153.15 && (parts[i].tmp == 1 || parts[i].tmp == 2)) //Sulfide Powders
	{
		if (parts[i].tmp == 1)
		{
			if (RNG::Ref().chance(1, 15))
			{
				sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_CAUS);
			}
			sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_SMKE);
		}

		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_STNE;
		parts[i].tmp = 2; //It must be in the "roasted" form regardless of pre-melting state
	}
	else if (parts[i].type == PT_ROCK && parts[i].temp >= 1387.15 && parts[i].tmp == 3) //Galena
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_ROCK;
		parts[i].tmp = 3;
	}
	else if (parts[i].type == PT_STNE && parts[i].temp >= 1387.15 && parts[i].tmp == 3) //Galena Powder
	{
		if (RNG::Ref().chance(1, 15))
		{
			sim->create_part(-1, x + RNG::Ref().chance(1, 3) - 2, y + RNG::Ref().chance(1, 3) - 2, PT_CAUS);
		}
		sim->part_change_type(i, x, y, PT_LAVA);
		if (RNG::Ref().chance(1, 5))
		{
			parts[i].tmp = 47; //Silver
			parts[i].ctype = PT_GOLD;
		}
		else if (RNG::Ref().chance(1, 5))
		{
			parts[i].tmp = 82; //Lead
			parts[i].ctype = PT_METL;
		}
		else
			parts[i].ctype = PT_GLAS;
	}
	else if (parts[i].type == PT_STNE && parts[i].temp >= 983.0f && (parts[i].tmp != 1 && parts[i].tmp != 2 && parts[i].tmp != 3)) //Regular STNE, no change
		sim->part_change_type(i, x, y, PT_LAVA);
	else if (parts[i].tmp == 82 && parts[i].type == PT_METL && parts[i].temp >= 600.65f) //Lead (Regular METL is higher melting temp, so no change or transition required here)
	{
		sim->part_change_type(i, x, y, PT_LAVA);
		parts[i].ctype = PT_METL;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp2 == 0) //Set random tmp2 vals if they do not exist, may be the case for particles transitioned to ROCK with PROP or console
		cpart->tmp2 = RNG::Ref().between(0, 10);
	int z = (cpart->tmp2 - 7) * 6; // Randomized color noise based on tmp2
	*colr += z;
	*colg += z;
	*colb += z;

	if (cpart->temp >= 810.15) //Glows when hot, right before melting becomes bright
	{
		*pixel_mode |= FIRE_ADD;

		*firea = int(((cpart->temp)-810.15)/20);
		*firer = (*colr);
		*fireg = (*colg/2);
	}
	if (cpart->tmp == 1) //Sulfide Color
	{
		*colr += 50;
		*colg += 30;
		*colb += -30;
	}
	else if (cpart->tmp == 2) //Roasted Sulfide Color
	{
		*colr += 25;
		*colg += 15;
		*colb += -15;
	}
	else if (cpart->tmp == 3) //Galena (Lead Sulfide) Color
	{
		int w = (cpart->tmp2 * 2);
		*colr = 84 + w;
		*colg = 84 + w;
		*colb = 84 + w;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 10);
}
