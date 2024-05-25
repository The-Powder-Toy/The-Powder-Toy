#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_RSSS()
{
	Identifier = "DEFAULT_PT_RSSS";
	Name = "RSSS";
	Colour = 0xC43626_rgb;
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
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 130;
	Description = "Solidified resist. Blocks pressure and insulates electricity. Liquefies on contact with neutrons.";

	Properties = TYPE_SOLID|PROP_NEUTPASS;
	CarriesTypeIn = (1U << FIELD_CTYPE) | (1U << FIELD_TMP);

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for(int rx = -1; rx < 2; rx++)
	{
		for(int ry = -1; ry < 2; ry++)
		{
			auto r = pmap[y+ry][x+rx];

			if (!r)
				continue;

			// Set RSSS ctype from nearby clone
			if((TYP(r) == PT_CLNE) || (TYP(r) == PT_PCLN))
			{
				if(parts[ID(r)].ctype != PT_RSSS)
					parts[i].ctype = parts[ID(r)].ctype;
			}

			// Set RSSS tmp from nearby breakable clone
			if((TYP(r) == PT_BCLN) || (TYP(r) == PT_PBCN))
			{
				if(parts[ID(r)].ctype != PT_RSSS)
					parts[i].tmp = parts[ID(r)].ctype;
			}
		}
	}

	//Block air like TTAN
	sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
	sim->air->bmap_blockairh[y/CELL][x/CELL] = 0x8;

	return 0;
}
