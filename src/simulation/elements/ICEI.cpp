#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ICEI()
{
	Identifier = "DEFAULT_PT_ICEI";
	Name = "ICE";
	Colour = PIXPACK(0xA0C0FF);
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
	HotAir = -0.0003f* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	DefaultProperties.temp = R_TEMP - 50.0f + 273.15f;
	HeatConduct = 46;
	Description = "Crushes under pressure. Cools down air.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 0.8f;
	HighPressureTransition = PT_SNOW;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 252.05f;
	HighTemperatureTransition = ST;

	DefaultProperties.ctype = PT_WATR;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].ctype==PT_FRZW)//get colder if it is from FRZW
	{
		parts[i].temp = restrict_flt(parts[i].temp-1.0f, MIN_TEMP, MAX_TEMP);
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SALT || TYP(r)==PT_SLTW)
				{
					if (parts[i].temp > sim->elements[PT_SLTW].LowTemperature && RNG::Ref().chance(1, 200))
					{
						sim->part_change_type(i,x,y,PT_SLTW);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SLTW);
						return 0;
					}
				}
				else if ((TYP(r)==PT_FRZZ) && RNG::Ref().chance(1, 200))
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_ICEI);
					parts[ID(r)].ctype = PT_FRZW;
				}
			}
	return 0;
}
