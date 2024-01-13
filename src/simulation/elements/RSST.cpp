#include "simulation/ElementCommon.h"

int update(UPDATE_FUNC_ARGS);

void Element::Element_RSST()
{
	Identifier = "DEFAULT_PT_RSST";
	Name = "RSST";
	Colour = 0xF95B49_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 50;

	Weight = 33;

	DefaultProperties.temp = R_TEMP + 20.0f + 273.15f;
	HeatConduct = 55;
	Description = "Resist. Solidifies on contact with photons, is destroyed by electrons and spark.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPASS;

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

int update(UPDATE_FUNC_ARGS)
{
	for(int rx = -1; rx < 2; rx++)
	{
		for(int ry = -1; ry < 2; ry++)
		{
			auto r = pmap[y+ry][x+rx];

			if (!r)
				continue;

			// RSST + GUNP = FIRW
			if(TYP(r) == PT_GUNP)
			{
				sim->part_change_type(i, x, y, PT_FIRW);
				sim->kill_part(ID(r));
				return 1;
			}

			// RSST + BCOL = FSEP
			if(TYP(r) == PT_BCOL)
			{
				sim->part_change_type(i, x, y, PT_FSEP);
				parts[i].life = 50;
				sim->kill_part(ID(r));
				return 1;
			}

			// Set RSST ctype from nearby clone
			if((TYP(r) == PT_CLNE) || (TYP(r) == PT_PCLN))
			{
				if(parts[ID(r)].ctype != PT_RSST)
					parts[i].ctype = parts[ID(r)].ctype;
			}
		}
	}

	return 0;
}
