#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_UVLT()
{
	Identifier = "DEFAULT_PT_UVLT";
	Name = "UVLT";
	Colour = 0x6c42f5_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	HeatConduct = 251;
	Description = "Ultraviolet light. Penetrates elements and splits them into their constituent parts.";

	Properties = TYPE_ENERGY | PROP_LIFE_DEC | PROP_LIFE_KILL_DEC | PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 3400;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	int under = pmap[y][x];
	int utype = TYP(under);
	if (!utype) return 0;

	int uID = ID(under);

	switch (utype) {
		case PT_TUNG: //TUNG absorbs UVLT
			parts[uID].temp = restrict_flt(parts[uID].temp + 2500.0f, MIN_TEMP, MAX_TEMP);
			sim->kill_part(i);
			return 1;
		case PT_LCRY: //"Fluorescence"
			if (parts[uID].life > 5) {
				sim->part_change_type(i, x, y, PT_PHOT);
				parts[i].ctype = 0x3FFFFFF0;
				return 1;
			}
			break;
		case PT_DEUT: //UVLT + DEUT -> WATR + GLOW
			parts[uID].life = 0;
			if (sim->rng.chance(1, 2))
				sim->part_change_type(uID, x, y, PT_DSTW);
			else
				sim->part_change_type(uID, x, y, PT_GLOW);
			break;
		case PT_RSST:
		case PT_RSSS: //UVLT + RSST -> GLOW + GEL
			if (sim->rng.chance(1, 2))
				sim->part_change_type(uID, x, y, PT_GLOW);
			else
				sim->part_change_type(uID, x, y, PT_GEL);
			break;
		case PT_BIZR:
		case PT_BIZRS:
		case PT_BIZRG: //UVLT + BIZR -> RSST
			sim->part_change_type(uID, x, y, PT_RSST);
			break;
		case PT_GLOW: //UVLT + GLOW -> BIZR (did this to add GEL production)
			if (sim->rng.chance(1, 100)) {
				sim->part_change_type(uID, x, y, PT_BIZR);
				auto c = sim->create_part(-3, x, y, PT_NEUT);
				parts[c].temp = 0.0f;
				parts[uID].temp = restrict_flt(parts[uID].temp - 100.0f, MIN_TEMP, MAX_TEMP);
			}
			break;
		case PT_ISOZ: //UVLT + ISOZ/ISZS -> ACID + NEUT + EXPLOSIONS
			if (sim->rng.chance(1, 2)) {
				sim->create_part(uID, x, y, PT_ACID);
				if (sim->rng.chance(1, 2)) {
					auto c = sim->create_part(-3, x, y, PT_NEUT);
					parts[c].temp = MAX_TEMP;
				}
				sim->pv[y/CELL][x/CELL] -= 15.0f;
				break;
			}
		case PT_ISZS:
			if (sim->rng.chance(1, 10)) {
				sim->create_part(uID, x, y, PT_UVLT);
				parts[uID].temp = restrict_flt(parts[uID].temp + 2000.0f, MIN_TEMP, MAX_TEMP);
				sim->pv[y/CELL][x/CELL] -= 15.0f;
			}
			break;
		case PT_CAUS: //UVLT + CAUS -> ACID
			if (sim->rng.chance(1, 10)) {
				sim->create_part(uID, x, y, PT_ACID);
				parts[uID].temp = 0.0f;
				parts[i].temp = 0.0f;
				sim->pv[y/CELL][x/CELL] += 1.0f;
			}
			break;
		case PT_SAWD: //UVLT + SAWD -> BCOL + CO2
			if (parts[uID].temp > 750.0f && sim->rng.chance(1, 3)) {
				if (sim->rng.chance(3, 4))
					sim->create_part(uID, x, y, PT_BCOL);
				else
					sim->create_part(uID, x, y, PT_CO2);
			}
			break;
		case PT_WOOD: //UVLT + WOOD -> COAL + CO2
			if (parts[uID].temp > 750.0f && sim->rng.chance(1, 3)) {
				if (sim->rng.chance(3, 4))
					sim->create_part(uID, x, y, PT_COAL);
				else
					sim->create_part(uID, x, y, PT_CO2);
			}
			break;
		case PT_PLNT: //UVLT + PLNT -> CO2
			if (parts[uID].temp > 400.0f && sim->rng.chance(1, 10))
				sim->create_part(uID, x, y, PT_CO2);
	}

	//UVLT will generate heat while inside other elements.
	if (utype != PT_WIFI)
		parts[uID].temp = restrict_flt(parts[uID].temp + 200.0f, MIN_TEMP, MAX_TEMP);

	//Absorption
	if (sim->rng.chance(1, 100)) {
		sim->kill_part(i);
		return 1;
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = 255;
	*colg = 255;
	*colb = 255;

	*firea = 130;
	*firer = 130;
	*fireg = 70;
	*fireb = 250;

	*pixel_mode |= FIRE_BLEND | NO_DECO;
	return 1;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	float a = sim->rng.between(0, 71) * 0.08727f;
	sim->parts[i].vx = 4.0f * cosf(a); //UVLT is faster than PHOT
	sim->parts[i].vy = 4.0f * sinf(a);
}
