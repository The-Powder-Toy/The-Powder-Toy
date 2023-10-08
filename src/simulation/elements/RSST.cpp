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

	Weight = 34;

	DefaultProperties.temp = R_TEMP + 20.0f + 273.15f;
	HeatConduct = 44;
	Description = "Resist. Solidifies on contact with PHOT, and reacts with other energy particles.";

	Properties = TYPE_LIQUID|PROP_NEUTPASS;

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

			if(TYP(r) == PT_GUNP)
			{
				sim->part_change_type(i, x, y, PT_FIRW);
				sim->kill_part(ID(r));
				return 1;
			}
		}
	}

	return 0;
}
