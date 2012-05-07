#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FIRW PT_FIRW 69
Element_FIRW::Element_FIRW()
{
    Identifier = "DEFAULT_PT_FIRW";
    Name = "FIRW";
    Colour = PIXPACK(0xFFA040);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;
    
    Advection = 0.7f;
    AirDrag = 0.02f * CFDS;
    AirLoss = 0.96f;
    Loss = 0.80f;
    Collision = -0.99f;
    Gravity = 0.1f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 30;
    
    Weight = 55;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 70;
    Description = "Fireworks!";
    
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
    
    Update = &Element_FIRW::update;
    Graphics = &Element_FIRW::graphics;
}

//#TPT-Directive ElementHeader Element_FIRW static int update(UPDATE_FUNC_ARGS)
int Element_FIRW::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, rt, np;
	if (parts[i].tmp==0) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = parts[r>>8].type;
					if (rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
					{
						parts[i].tmp = 1;
						parts[i].life = rand()%50+60;
					}
				}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life==0) {
			parts[i].tmp=2;
		} else {
			float newVel = parts[i].life/25;
			parts[i].flags &= ~FLAG_STAGNANT;
			/* TODO:
			if ((pmap[(int)(ly-newVel)][(int)lx]&0xFF)==PT_NONE && ly-newVel>0) {
				parts[i].vy = -newVel;
				ly-=newVel;
				iy-=newVel;
			}*/
			parts[i].vy = -newVel;
		}
	}
	else if (parts[i].tmp==2) {
		int col = rand()%200+4;
		int tmul;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					tmul = rand()%7;
					np = sim->create_part(-1, x+rx, y+ry, PT_FIRW);
					if (np>-1)
					{
						parts[np].vx = (rand()%3-1)*tmul;
						parts[np].vy = (rand()%3-1)*tmul;
						parts[np].tmp = col;
						parts[np].life = rand()%100+100;
						parts[np].temp = 6000.0f;
						parts[np].dcolour = parts[i].dcolour;
					}
				}
		sim->pv[y/CELL][x/CELL] += 20;
		sim->kill_part(i);
		return 1;
	} else if (parts[i].tmp>=3) {
		if (parts[i].life<=0) {
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_FIRW static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FIRW::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->tmp>=3)
	{
		int caddress = restrict_flt(restrict_flt((float)(cpart->tmp-4), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
		*colr = 0;//(unsigned char)firw_data[caddress];
		*colg = 0;//(unsigned char)firw_data[caddress+1];
		*colb = 0;//(unsigned char)firw_data[caddress+2];
		
		if (ren->decorations_enable && cpart->dcolour)
		{
			int a = (cpart->dcolour>>24)&0xFF;
			*colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
			*colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
			*colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
		}
		
		*firea = cpart->life*4;
		if(*firea > 240)
			*firea = 240;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		
		*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
		*pixel_mode |= FIRE_ADD;
		//Returning 0 means dynamic, do not cache
	}
	else if(cpart->tmp > 0)
	{
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}


Element_FIRW::~Element_FIRW() {}