#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RBBR PT_RBBR 214
Element_RBBR::Element_RBBR()
{
	Identifier = "DEFAULT_PT_RBBR";
	Name = "RBBR";
	Colour = PIXPACK(0xFF99FF);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 0;
	
	Flammable = 2;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 100;
	Description = "Rubber, reverses velocity of particles";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 682.15;
	HighTemperatureTransition = PT_GEL;
	
	Update = &Element_RBBR::update;
}
//#TPT-Directive ElementHeader Element_RBBR static int update(UPDATE_FUNC_ARGS)
int Element_RBBR::update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry;
	for(rx=-1; rx<2; rx++)
		for(ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				continue;
				if(parts[i].temp<=682.15 && parts[i].temp>273.15 && (r&0xFF)!=PT_RBBR && sim->elements[r>>8].Properties != TYPE_SOLID && (r&0xFF) != PT_SPRK)
				{
					int W = sim->elements[r>>8].Weight;
					int C = sim->elements[r>>8].Collision*10;
					parts[r>>8].vx=(parts[r>>8].vx-(1/parts[i].temp))*C*W;
					parts[r>>8].vy=(parts[r>>8].vy-(1/parts[i].temp))*C*W;
				}
				else if(parts[i].temp<=273.15)
				{
					return 0;
				}
			}
	return 0;
}

Element_RBBR::~Element_RBBR() {}
