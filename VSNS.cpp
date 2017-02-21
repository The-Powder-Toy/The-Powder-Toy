#include "simulation/Elements.h"
Element_VSNS::Element_VSNS()
{
	Identifier = "DEFAULT_PT_VSNS";
	Name = "VSNS";
	Colour = PIXPACK(0xFD9D18);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
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
	Description = "Speed sensor. Generates sparks when a particle has a higher velocity than its temp.";

	Properties = TYPE_SOLID|PROP_DRAWONCTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_VSNS::update;
}

int Element_VSNS::update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	float v;
	for(rx=-1;rx<2;rx++)
	{
		for(ry=-1;ry<2;ry++)
		{
			if((BOUNDS_CHECK)&&(rx||ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
				{
					continue;
				}
				v = parts[r>>8].vx + parts[r>>8].vy;
				if((v >= parts[i].temp/10))
				{
					if(parts[i].ctype == r&0xFF)
					{
						parts[i].life=1;
					}
					else if(!parts[i].ctype)
					{
						parts[i].life=1;
					};
				}
				if(parts[i].life)
				{
					parts[i].life=0;
					rt = r&0xFF;
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
			}
		}
	}
	return 0;
}

Element_VSNS::~Element_VSNS() {}
