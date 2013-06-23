#include "simulation/Elements.h"

Element_STAR::Element_STAR()
{
	Identifier = "DEFAULT_PT_STAR";
	Name = "STAR";
	Colour = PIXPACK(0xCCCCCC);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Hardness = 0;
	
	Weight = 100;
	
	Temperature = R_TEMP+MAX_TEMP;
	HeatConduct = 251;
	Description = "Star. Still a bit buggy at the moment.";
	
	State = ST_SOLID;
	Properties = TYPE_PART|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_STAR::update;
	Graphics = &Element_STAR::graphics;
}


int Element_STAR::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				{
					if(r&0xFF){
						sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = 2.5f;
						parts[i].temp = MAX_TEMP;
					}
					if ((r&0xFF) && (r&0xFF) != PT_SDST && (r&0xFF) !=PT_NONE && (r&0xFF) !=PT_STAR && (r&0xFF) && (r&0xFF) != PT_PLSM)
						{
							parts[r>>8].type = PT_PLSM;
						}
					if((r&0xFF)==PT_STAR){
						parts[i].life++;
					}
					if(parts[i].life==50000){
					parts[i].type=PT_SING;
					parts[i].life=0;
					parts[i].tmp=99999;  //go boom
				}
				}
		}
	return 0;
}


int Element_STAR::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_STAR::~Element_STAR() {}
