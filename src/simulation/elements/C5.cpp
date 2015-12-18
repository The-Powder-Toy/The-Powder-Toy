#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_C5 PT_C5 130
Element_C5::Element_C5()
{
	Identifier = "DEFAULT_PT_C5";
	Name = "C-5";
	Colour = PIXPACK(0x2050E0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 88;
	Description = "Cold explosive, set off by anything cold.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_C5::update;
}

//#TPT-Directive ElementHeader Element_C5 static int update(UPDATE_FUNC_ARGS)
int Element_C5::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)!=PT_C5 && parts[r>>8].temp<100 && sim->elements[r&0xFF].HeatConduct && ((r&0xFF)!=PT_HSWC||parts[r>>8].life==10)) || (r&0xFF)==PT_CFLM)
				{
					if (!(rand()%6))
					{
						sim->part_change_type(i,x,y,PT_CFLM);
						parts[r>>8].temp = parts[i].temp = 0;
						parts[i].life = rand()%150+50;
						sim->pv[y/CELL][x/CELL] += 1.5;
					}
				}
			}
	return 0;
}


Element_C5::~Element_C5() {}
