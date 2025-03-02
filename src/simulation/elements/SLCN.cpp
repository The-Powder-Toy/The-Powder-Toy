#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_SLCN()
{
	Identifier = "DEFAULT_PT_SLCN";
	Name = "SLCN";
	Colour = 0xBCCDDF_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.27f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	HeatConduct = 100;
	Description = "Powdered Silicon. A key ingredient in producing multiple materials.";

	Properties = TYPE_PART | PROP_CONDUCTS | PROP_HOT_GLOW | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3538.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static const RGB SLCN_COLOUR[16] = {
	0x5A6679_rgb, 0x6878A1_rgb, 0xABBFDD_rgb, 0x838490_rgb,
	0xBCCDDF_rgb, 0x82A0D2_rgb, 0x5B6680_rgb, 0x232C3B_rgb,
	0x485067_rgb, 0x8B9AB6_rgb, 0xADB1C1_rgb, 0xC3C6D1_rgb,
	0x8594AD_rgb, 0x262F47_rgb, 0xA9AEBC_rgb, 0xC2E1F7_rgb,
};

static void initSparkles(Simulation *sim, Particle &part)
{
	// bits 31-20: phase increment (randomised to a value between 1 and 9)
	// bits 19-16: next colour index
	// bits 15-12: current colour index
	// bits 11-00: phase
	part.tmp = sim->rng.between(0x100000, 0x9FFFFF);
}

static int update(UPDATE_FUNC_ARGS)
{
	if (!parts[i].tmp)
	{
		initSparkles(sim, parts[i]);
	}
	int phase = (parts[i].tmp & 0xFFF) + ((parts[i].tmp >> 20) & 0xFFF);
	if (phase & 0x1000)
	{
		// discard current, current <- next, next <- random, wrap phase
		parts[i].tmp = (parts[i].tmp & 0xFFF00000) | (phase & 0xFFF) | (sim->rng.between(0, 15) << 16) | ((parts[i].tmp >> 4) & 0xF000);
	}
	else
	{
		// update phase
		parts[i].tmp = (parts[i].tmp & 0xFFFFF000) | phase;
	}

	if (parts[i].life == 0 && parts[i].temp < 373.15f)
	{
		for (int j = 0; j < 4; j++)
		{
			static const int check_coords_x[] = { -4, 4, 0, 0 };
			static const int check_coords_y[] = { 0, 0, -4, 4 };
			int n = pmap[y + check_coords_y[j]][x + check_coords_x[j]];
			if (n && TYP(n) == PT_SPRK)
			{
				Particle &neighbour = parts[ID(n)];
				if (neighbour.life != 0 && neighbour.life < 4)
				{
					sim->part_change_type(i, x, y, PT_SPRK);
					parts[i].life = 4;
					parts[i].ctype = PT_SLCN;
				}
			}
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	RGB curr_colour = SLCN_COLOUR[(cpart->tmp >> 12) & 15];
	if (cpart->tmp & 0x800) // mix with next colour if phase is at least halfway there
	{
		RGB next_colour = SLCN_COLOUR[(cpart->tmp >> 16) & 15];
		curr_colour = RGB(
			(curr_colour.Red   + next_colour.Red) / 2,
			(curr_colour.Green + next_colour.Green) / 2,
			(curr_colour.Blue  + next_colour.Blue) / 2
		);
	}
	*colr = curr_colour.Red;
	*colg = curr_colour.Green;
	*colb = curr_colour.Blue;

	int rnd = (cpart->tmp & 0xFFFF) * ((cpart->tmp >> 16) & 0xFFFF);
	if (!(rnd % 887))
	{
		*pixel_mode |= PMODE_FLARE | PMODE_GLOW;
	}
	if (!(rnd % 593))
	{
		*pixel_mode |= PMODE_SPARK;
	}

	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	initSparkles(sim, sim->parts[i]);
}
