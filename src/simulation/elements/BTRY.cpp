#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BTRY()
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

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry) && abs(rx)+abs(ry)<4)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = TYP(r);
				if (sim->parts_avg(i,ID(r),PT_INSL) != PT_INSL)
				{
					if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[ID(r)].life==0)
					{
						parts[ID(r)].life = 4;
						parts[ID(r)].ctype = rt;
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
					}
				}
			}
	return 0;
}
