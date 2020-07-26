#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_PTNM()
{
	Identifier = "DEFAULT_PT_PTNM";
	Name = "PTNM";
	Colour = PIXPACK(0xD5E0EB);
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
	Hardness = 0;
	Weight = 100;

	HeatConduct = 251;
	Description = "Platinum. Catalyzes certain reactions.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW | PROP_SPARKSETTLE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1768.0f + 273.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static void hygn_reactions(int hygn1_id, UPDATE_FUNC_ARGS)
{
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r || ID(r) == hygn1_id)
					continue;
				int rt = TYP(r);

				// HYGN + DESL -> OIL + WATR
				if (rt == PT_DESL)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_WATR);
					sim->part_change_type(hygn1_id, (int)(parts[hygn1_id].x + 0.5f), (int)(parts[hygn1_id].y + 0.5f), PT_OIL);
					return;
				}

				// HYGN + OXYG -> DSTW + SPRK + Heat
				if (rt == PT_O2 && !parts[i].life)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_DSTW);
					sim->part_change_type(hygn1_id, (int)(parts[hygn1_id].x + 0.5f), (int)(parts[hygn1_id].y + 0.5f), PT_DSTW);
					parts[ID(r)].temp += 5.0f;
					parts[hygn1_id].temp += 5.0f;

					parts[i].ctype = PT_PTNM;
					parts[i].life = 4;
					sim->part_change_type(i, x, y, PT_SPRK);
					return;
				}

				// Cold fusion: 2 hydrogen > 500 C has a chance to fuse
				if (rt == PT_H2 && RNG::Ref().chance(1, 1000) && parts[ID(r)].temp > 500.0f + 273.15f && parts[hygn1_id].temp > 500.0f + 273.15f)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_NBLE);
					sim->part_change_type(hygn1_id, (int)(parts[hygn1_id].x + 0.5f), (int)(parts[hygn1_id].y + 0.5f), PT_NEUT);

					parts[ID(r)].temp += 1000.0f;
					parts[hygn1_id].temp += 1000.0f;
					sim->pv[y / CELL][x / CELL] += 10.0f;

					int j = sim->create_part(-3, x + rx, y + ry, PT_PHOT);
					if (j > -1)
					{
						parts[j].ctype = 0x7C0000;
						parts[j].temp = parts[ID(r)].temp;
						parts[j].tmp = 0x1;
					}
					if (RNG::Ref().chance(1, 10))
					{
						int j = sim->create_part(-3, x + rx, y + ry, PT_ELEC);
						if (j > -1)
							parts[j].temp = parts[ID(r)].temp;
					}
					return;
				}
			}
		}
	}
}

static int update(UPDATE_FUNC_ARGS)
{
	int hygn1_id = -1; // Id of a hydrogen particle for hydrogen multi-particle reactions

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
				parts[i].ctype = PT_PTNM;
			}
		}
	}

	// Single element reactions
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				int rt = TYP(r);

				if (rt == PT_H2 && hygn1_id < 0)
					hygn1_id = ID(r);

				// These reactions will occur instantly in contact with PTNM
				// --------------------------------------------------------

				// Shield instantly grows (even without SPRK)
				if (!parts[ID(r)].life && (rt == PT_SHLD1 || rt == PT_SHLD2 || rt == PT_SHLD3))
				{
					int next = PT_SHLD1;
					switch (rt)
					{
					case PT_SHLD1: next = PT_SHLD2; break;
					case PT_SHLD2: next = PT_SHLD3; break;
					case PT_SHLD3: next = PT_SHLD4; break;
					}
					sim->part_change_type(ID(r), x + rx, y + ry, next);
					parts[ID(r)].life = 7;
					continue;
				}

				// ISZS / ISOZ -> PHOT + PLUT
				if (rt == PT_ISZS || rt == PT_ISOZ)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_PLUT);
					sim->create_part(-3, x + rx, y + ry, PT_PHOT);
					continue;
				}

				// These reactions are dependent on temperature
				// Probability goes quadratically from 0% / frame to 100% / frame from 0 C to 1500 C
				// --------------------------------------------------------
				float prob = std::min(1.0f, parts[i].temp / (273.15f + 1500.0f));
				prob *= prob;

				if (RNG::Ref().uniform01() <= prob)
				{
					switch (rt)
					{
					case PT_GAS: // GAS + > 2 pressure + >= 200 C -> INSL
						if (parts[ID(r)].temp >= 200.0f + 273.15f && sim->pv[y / CELL][x / CELL] > 2.0f)
						{
							sim->part_change_type(ID(r), x + rx, y + ry, PT_INSL);
							parts[i].temp += 60.0f; // Other part is INSL, adding temp is useless
						}
						break;

					case PT_BREC: // BREL + > 1000 C + > 50 pressure -> EXOT
						if (parts[ID(r)].temp > 1000.0f + 273.15f && sim->pv[y / CELL][x / CELL] > 50.0f)
						{
							sim->part_change_type(ID(r), x + rx, y + ry, PT_EXOT);
							parts[ID(r)].temp -= 30.0f;
							parts[i].temp -= 30.0f;
						}
						break;

					case PT_SMKE: // SMKE -> CO2
						sim->part_change_type(ID(r), x + rx, y + ry, PT_CO2);
						break;
					}
				}
			}
		}
	}

	// Hydrogen reactions
	if (hygn1_id >= 0)
	{
		hygn_reactions(hygn1_id, UPDATE_FUNC_SUBCALL_ARGS);
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp)
		*pixel_mode |= PMODE_FLARE;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	if (RNG::Ref().chance(1, 15))
		sim->parts[i].tmp = 1;
}
