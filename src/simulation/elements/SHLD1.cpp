#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SHLD1()
{
	Identifier = "DEFAULT_PT_SHLD1";
	Name = "SHLD";
	Colour = PIXPACK(0xAAAAAA);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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

	HeatConduct = 0;
	Description = "Shield, spark it to grow.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 7.0f;
	HighPressureTransition = PT_NONE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, nnx, nny, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if (TYP(r)==PT_SPRK&&parts[i].life==0)
				{
					if (RNG::Ref().chance(11, 40))
					{
						sim->part_change_type(i,x,y,PT_SHLD2);
						parts[i].life = 7;
					}
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								//parts[ID(pmap[y+ny+nny][x+nx+nnx])].life=7;
							}
						}
				}
				else if (TYP(r) == PT_SHLD3 && RNG::Ref().chance(2, 5))
				{
					sim->part_change_type(i,x,y,PT_SHLD2);
					parts[i].life = 7;
				}
			}
	return 0;
}
