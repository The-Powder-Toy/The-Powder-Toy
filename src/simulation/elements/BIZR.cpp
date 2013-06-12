#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BIZR PT_BIZR 103
Element_BIZR::Element_BIZR()
{
	Identifier = "DEFAULT_PT_BIZR";
	Name = "BIZR";
	Colour = PIXPACK(0x00FF77);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	
	Weight = 30;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 29;
	Description = "Bizarre... contradicts the normal state changes. Paints other elements with its deco color.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 100.0f;
	LowTemperatureTransition = PT_BIZRG;
	HighTemperature = 400.0f;
	HighTemperatureTransition = PT_BIZRS;
	
	Update = &Element_BIZR::update;
	Graphics = &Element_BIZR::graphics;
}

//#TPT-Directive ElementHeader Element_BIZR static int update(UPDATE_FUNC_ARGS)
int Element_BIZR::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	float blend;
	if(parts[i].dcolour){
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
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
	if(((r = sim->photons[y][x])&0xFF)==PT_PHOT)//this should be in movement checks?
	{
		sim->part_change_type(r>>8, x, y, PT_ELEC);
		parts[r>>8].ctype = 0;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_BIZR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_BIZR::graphics(GRAPHICS_FUNC_ARGS)
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
	x = *colr+*colg+*colb+1;
	*colr = *colr*624/x;
	*colg = *colg*624/x;
	*colb = *colb*624/x;
	if(fabs(cpart->vx)+fabs(cpart->vy)>0)
	{
		*firea = 255;
		*fireg = *colg/5 * (fabs(cpart->vx)+fabs(cpart->vy));
		*fireb = *colb/5 * (fabs(cpart->vx)+fabs(cpart->vy));
		*firer = *colr/5 * (fabs(cpart->vx)+fabs(cpart->vy));
		*pixel_mode |= FIRE_ADD;
	}
	*pixel_mode |= PMODE_BLUR;
	return 0;
}


Element_BIZR::~Element_BIZR() {}
