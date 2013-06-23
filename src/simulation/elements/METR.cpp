#include "simulation/Elements.h"

Element_METR::Element_METR()
{
	Identifier = "DEFAULT_PT_METR";
	Name = "METR";
	Colour = PIXPACK(0xE8A71C);
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
	Description = "Meteor. Yes, this is a moving solid.";
	
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
	
	Update = &Element_METR::update;
	Graphics = &Element_METR::graphics;
}


int Element_METR::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
	int flags;
	int l = 3;
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
					sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] += 0.1f;
					parts[i].temp = MAX_TEMP;
				}
				if (((r&0xFF)==PT_NONE))
                {
                    sim->create_part(i,x,y,PT_SDST);
                }
                else
                {
                    sim->create_part(i,x,y,PT_METR);
                }
                if ((r&0xFF) && (r&0xFF) != PT_SDST && (r&0xFF) !=PT_NONE && (r&0xFF) !=PT_METR && (r&0xFF) && (r&0xFF) != PT_PLSM)
                    {
                        parts[r>>8].type = PT_PLSM;
                    }
				if(((r&0xFF)!=PT_NONE) && ((r&0xFF)!=PT_METR) && ((r&0xFF)!=PT_LAVA) && ((r&0xFF)!=PT_PLSM) && ((r&0xFF)!=PT_SMKE) && ((r&0xFF)!=PT_FIRE)){
					parts[i].type = PT_LRVS;
					sim->CreateParts(x,y,rx,ry,PT_THRM,flags);
					parts[r>>8].vy -= 2.0f;
				}
					parts[i].y += l;
				}
		}
	return 0;
}


int Element_METR::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_METR::~Element_METR() {}
