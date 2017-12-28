#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FRZW PT_FRZW 101
Element_FRZW::Element_FRZW()
{
	Identifier = "DEFAULT_PT_FRZW";
	Name = "FRZW";
	Colour = PIXPACK(0x1020C0);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	Temperature = 120.0f;
	HeatConduct = 29;
	Description = "Freeze water. Hybrid liquid formed when Freeze powder melts.";

	Properties = TYPE_LIQUID | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 53.0f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FRZW::update;
}

//#TPT-Directive ElementHeader Element_FRZW static int update(UPDATE_FUNC_ARGS)
int Element_FRZW::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_WATR && !(rand()%14))
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_FRZW);
				}
			}
	if ((parts[i].life==0 && !(rand()%192)) || (100-(parts[i].life))>rand()%50000 )
	{
		sim->part_change_type(i,x,y,PT_ICEI);
		parts[i].ctype=PT_FRZW;
		parts[i].temp = restrict_flt(parts[i].temp-200.0f, MIN_TEMP, MAX_TEMP);
	}
	return 0;
}


Element_FRZW::~Element_FRZW() {}
