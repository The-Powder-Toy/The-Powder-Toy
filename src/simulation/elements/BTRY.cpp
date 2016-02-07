#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BTRY PT_BTRY 53
Element_BTRY::Element_BTRY()
{
	Identifier = "DEFAULT_PT_BTRY";
	Name = "BTRY";
	Colour = PIXPACK(0x858505);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	HeatConduct = 251;
	Description = "Battery. Generates infinite electricity.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2273.0f;
	HighTemperatureTransition = PT_PLSM;

	Update = &Element_BTRY::update;
}

//#TPT-Directive ElementHeader Element_BTRY static int update(UPDATE_FUNC_ARGS)
int Element_BTRY::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry) && abs(rx)+abs(ry)<4)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = (r&0xFF);
				if (sim->parts_avg(i,r>>8,PT_INSL) != PT_INSL)
				{
					if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0)
					{
						parts[r>>8].life = 4;
						parts[r>>8].ctype = rt;
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					}
				}
			}
	return 0;
}


Element_BTRY::~Element_BTRY() {}
