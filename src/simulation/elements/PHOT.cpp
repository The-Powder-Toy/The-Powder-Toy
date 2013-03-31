#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PHOT PT_PHOT 31
Element_PHOT::Element_PHOT()
{
	Identifier = "DEFAULT_PT_PHOT";
	Name = "PHOT";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = -1;
	
	Temperature = R_TEMP+900.0f+273.15f;
	HeatConduct = 251;
	Description = "Photons. Travel in straight lines.";
	
	State = ST_GAS;
	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_PHOT::update;
	Graphics = &Element_PHOT::graphics;
}

//#TPT-Directive ElementHeader Element_PHOT static int update(UPDATE_FUNC_ARGS)
int Element_PHOT::update(UPDATE_FUNC_ARGS)
 {
	int r, rt, rx, ry;
	float rr, rrr;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	if (!(parts[i].ctype&0x3FFFFFFF)) {
		sim->kill_part(i);
		return 1;
	}
	if (parts[i].temp > 506)
		if (!(rand()%10)) Element_FIRE::update(UPDATE_FUNC_SUBCALL_ARGS);
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_ISOZ || (r&0xFF)==PT_ISZS)
				{
					if (!(rand()%400))
					{
						parts[i].vx *= 0.90;
						parts[i].vy *= 0.90;
						sim->create_part(r>>8, x+rx, y+ry, PT_PHOT);
						rrr = (rand()%360)*3.14159f/180.0f;
						rr = (rand()%128+128)/127.0f;
						parts[r>>8].vx = rr*cosf(rrr);
						parts[r>>8].vy = rr*sinf(rrr);
						sim->pv[y/CELL][x/CELL] -= 15.0f * CFDS;
					}
				}
				else if((r&0xFF) == PT_QRTZ && !ry && !rx)//if on QRTZ
				{
					float a = (rand()%360)*3.14159f/180.0f;
					parts[i].vx = 3.0f*cosf(a);
					parts[i].vy = 3.0f*sinf(a);
					if(parts[i].ctype == 0x3FFFFFFF)
						parts[i].ctype = 0x1F<<(rand()%26);
					parts[i].life++; //Delay death
				}
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_PHOT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PHOT::graphics(GRAPHICS_FUNC_ARGS)

{
	int x = 0;
	*colr = *colg = *colb = 0;
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

	*firea = 100;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD;
	return 0;
}


Element_PHOT::~Element_PHOT() {}
