#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SHLD2 PT_SHLD2 120
Element_SHLD2::Element_SHLD2()
{
	Identifier = "DEFAULT_PT_SHLD2";
	Name = "SHD2";
	Colour = PIXPACK(0x777777);
	MenuVisible = 0;
	MenuSection = SC_CRACKER2;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
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
	HeatConduct = 0;
	Description = "Shield lvl 2.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_NONE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_SHLD2::update;
}

//#TPT-Directive ElementHeader Element_SHLD2 static int update(UPDATE_FUNC_ARGS)
int Element_SHLD2::update(UPDATE_FUNC_ARGS)
{
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (parts[i].life>0)
						sim->create_part(-1,x+rx,y+ry,PT_SHLD1);
					continue;
				}
				else if (TYP(r)==PT_SPRK&&parts[i].life==0)
				{
					if (!(rand()%8))
					{
						sim->part_change_type(i,x,y,PT_SHLD3);
						parts[i].life = 7;
					}
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								np = sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
				}
				else if (TYP(r)==PT_SHLD4&&2>rand()%5)
				{
					sim->part_change_type(i,x,y,PT_SHLD3);
					parts[i].life = 7;
				}
			}
	return 0;
}



Element_SHLD2::~Element_SHLD2() {}
