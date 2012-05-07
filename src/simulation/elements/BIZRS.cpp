#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BIZRS PT_BIZRS 105
Element_BIZRS::Element_BIZRS()
{
    Identifier = "DEFAULT_PT_BIZRS";
    Name = "BIZS";
    Colour = PIXPACK(0x00E455);
    MenuVisible = 1;
    MenuSection = SC_CRACKER2;
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
    Meltable = 1;
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+300.0f+273.15f;
    HeatConduct = 251;
    Description = "Bizarre solid";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = 400.0f;
    LowTemperatureTransition = PT_BIZR;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_BIZRS::update;
    Graphics = &Element_BIZRS::graphics;
}

//#TPT-Directive ElementHeader Element_BIZRS static int update(UPDATE_FUNC_ARGS)
int Element_BIZRS::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	float blend;
	if(parts[i].dcolour){
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_BIZR && (r&0xFF)!=PT_BIZRG  && (r&0xFF)!=PT_BIZRS)
					{
						blend = 0.95f;
						tr = (parts[r>>8].dcolour>>16)&0xFF;
						tg = (parts[r>>8].dcolour>>8)&0xFF;
						tb = (parts[r>>8].dcolour)&0xFF;
						ta = (parts[r>>8].dcolour>>24)&0xFF;
						
						mr = (parts[i].dcolour>>16)&0xFF;
						mg = (parts[i].dcolour>>8)&0xFF;
						mb = (parts[i].dcolour)&0xFF;
						ma = (parts[i].dcolour>>24)&0xFF;
						
						nr = (tr*blend) + (mr*(1-blend));
						ng = (tg*blend) + (mg*(1-blend));
						nb = (tb*blend) + (mb*(1-blend));
						na = (ta*blend) + (ma*(1-blend));
						
						parts[r>>8].dcolour = nr<<16 | ng<<8 | nb | na<<24;
					}
				}
	}
	if(((r = sim->photons[y][x])&0xFF)==PT_PHOT || ((r = pmap[y][x])&0xFF)==PT_PHOT)
	{
		sim->part_change_type(r>>8, x, y, PT_ELEC);
		parts[r>>8].ctype = 0;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_BIZRS static int graphics(GRAPHICS_FUNC_ARGS)
int Element_BIZRS::graphics(GRAPHICS_FUNC_ARGS)
 //BIZR, BIZRG, BIZRS
{
	int x = 0;
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;
	if(fabs(cpart->vx)+fabs(cpart->vy)>0)
	{
		*firea = 255;
		*fireg = *colg/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*fireb = *colb/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*firer = *colr/5 * fabs(cpart->vx)+fabs(cpart->vy);
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}


Element_BIZRS::~Element_BIZRS() {}