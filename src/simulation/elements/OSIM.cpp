#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_OSIM()
{
	Identifier = "DEFAULT_PT_OSIM";
	Name = "OSIM";
	Colour = 0x769EE3_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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
	Meltable = 1;
	Hardness = 1;
	Weight = 100;

	HeatConduct = 251;
	Description = "Osmium. Shiny, blocks pressue, and is moderately strong.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW | PROP_SPARKSETTLE;
	PhotonReflectWavelengths = 0x02800FFF;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3033.0f + 273.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Fast conduction (like GOLD)
	if (!parts[i].life)
	{
		for (int j = 0; j < 4; j++)
		{
			static const int checkCoordsX[] = { -4, 4, 0, 0 };
			static const int checkCoordsY[] = { 0, 0, -4, 4 };
			int rx = checkCoordsX[j];
			int ry = checkCoordsY[j];
			int r = pmap[y + ry][x + rx];
			if (r && TYP(r) == PT_SPRK && parts[ID(r)].life && parts[ID(r)].life < 4)
			{
				sim->part_change_type(i, x, y, PT_SPRK);
				parts[i].life = 4;
				parts[i].ctype = PT_OSIM;
			}
		}
	}

	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				if (sim->rng.chance(1, 20))
				{
					if (TYP(r) == PT_O2) { // oxygen is less reactive
						if (sim->legacy_enable || parts[i].temp > (273.15f + 400.0f)) {
							sim->part_change_type(i, x, y, PT_OSTO);
							if (sim->rng.chance(1, 2)) {
								sim->part_change_type(ID(r), x + rx, y + ry, PT_OSTO);
							}
						}
						else if (sim->rng.chance(1, 25)) {
							sim->part_change_type(i, x, y, PT_OSTO);
						}
					}
					else if (TYP(r) == PT_WATR) { // water is much more reactive
						sim->part_change_type(i, x, y, PT_OSTO);
						sim->part_change_type(ID(r), x + rx, y + ry, PT_OSTO);
					}
				}

				if (TYP(r) == PT_ISOZ) {
					sim->part_change_type(i, x, y, PT_POLO);
					sim->parts[i].vx = sim->rng.between(-5, 5);
					sim->parts[i].vy = sim->rng.between(-5, 5);
					if (sim->rng.chance(1, 3)) {
						sim->part_change_type(ID(r), x + rx, y + ry, PT_POLO);
						sim->parts[ID(r)].vx = sim->rng.between(-5, 5);
						sim->parts[ID(r)].vy = sim->rng.between(-5, 5);
					}
				}

				else if ((TYP(r) == PT_RBDM || TYP(r) == PT_LRBD) && (sim->legacy_enable || parts[i].temp > (273.15f + 200.0f)) && sim->rng.chance(1, 100))
				{
					sim->part_change_type(i, x, y, PT_FIRE);
					parts[i].life = 4;
					parts[i].ctype = PT_WATR;
				}
			}
		}
	}

	sim->air->bmap_blockair[y / CELL][x / CELL] = 1;
	sim->air->bmap_blockairh[y / CELL][x / CELL] = 0x8;

	// add to the shininess
	if (sim->rng.chance(1, 30)) {
		sim->parts[i].tmp2 = sim->rng.between(0, 10);
	}
	
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 5) * 3;
	*colr += z;
	*colg += z;
	*colb += z;
	
	if (cpart->tmp)
	{
		*pixel_mode |= PMODE_FLARE;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = sim->rng.between(0, 10);
	if (sim->rng.chance(1, 15))
		sim->parts[i].tmp = 1;
}
