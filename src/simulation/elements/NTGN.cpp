#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NTGN PT_NTGN 173
Element_NTGN::Element_NTGN()
{
	Identifier = "DEFAULT_PT_NTGN";
	Name = "NTGN";
	Colour = PIXPACK(0x000099);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;
	
	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = -0.04f;
	Diffusion = 2.0f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = 263.15;
	HeatConduct = 200;
	Description = "Nitrogen";
	
	State = ST_GAS;
	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 63.15;
	LowTemperatureTransition = PT_LNTG;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_NTGN::update;
}

//#TPT-Directive ElementHeader Element_NTGN static int update(UPDATE_FUNC_ARGS)
int Element_NTGN::update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	for(rx=-1; rx<2; rx++)
		for(ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				continue;

				if((r&0xFF)==PT_H2 && parts[i].tmp==1)
				{
					sim->part_change_type(i,x,y,PT_RIME);
					sim->kill_part(r>>8);
				}
			}
			return 0;
}
Element_NTGN::~Element_NTGN() {}