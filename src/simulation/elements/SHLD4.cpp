#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SHLD4 PT_SHLD4 122
Element_SHLD4::Element_SHLD4()
{
	Identifier = "DEFAULT_PT_SHLD4";
	Name = "SHD4";
	Colour = PIXPACK(0x212121);
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
	Description = "Shield lvl 4.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 40.0f;
	HighPressureTransition = PT_NONE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_SHLD4::update;
}

//#TPT-Directive ElementHeader Element_SHLD4 static int update(UPDATE_FUNC_ARGS)
int Element_SHLD4::update(UPDATE_FUNC_ARGS)
{
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (!(rand()%5500))
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_SHLD1);
						if (np<0) continue;
						parts[np].life=7;
						sim->part_change_type(i,x,y,PT_SHLD2);
					}
					continue;

				}
				if ((r&0xFF)==PT_SHLD2 && parts[i].life>3)
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SHLD3);
					parts[r>>8].life = 7;
				}
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
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
	return 0;
}


Element_SHLD4::~Element_SHLD4() {}
