#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SHLD4()
{
	Identifier = "DEFAULT_PT_SHLD4";
	Name = "SHD4";
	Colour = 0x212121_rgb;
	MenuVisible = 0;
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

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (sim->rng.chance(1, 5500))
					{
						auto np = sim->create_part(-1,x+rx,y+ry,PT_SHLD1);
						if (np<0) continue;
						parts[np].life=7;
						sim->part_change_type(i,x,y,PT_SHLD2);
					}
					continue;

				}
				if (TYP(r)==PT_SHLD2 && parts[i].life>3)
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SHLD3);
					parts[ID(r)].life = 7;
				}
				else if (TYP(r)==PT_SPRK&&parts[i].life==0)
				{
					for (auto nnx = -1; nnx <= 1; nnx++)
					{
						for (auto nny = -1; nny <= 1; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								auto np = sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
