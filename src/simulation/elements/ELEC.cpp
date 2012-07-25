#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ELEC PT_ELEC 136
Element_ELEC::Element_ELEC()
{
    Identifier = "DEFAULT_PT_ELEC";
    Name = "ELEC";
    Colour = PIXPACK(0xDFEFFF);
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
    
    Temperature = R_TEMP+200.0f+273.15f;
    HeatConduct = 251;
    Description = "Electrons";
    
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
    
    Update = &Element_ELEC::update;
    Graphics = &Element_ELEC::graphics;
}

//#TPT-Directive ElementHeader Element_ELEC static int update(UPDATE_FUNC_ARGS)
int Element_ELEC::update(UPDATE_FUNC_ARGS)
 {
	int r, rt, rx, ry, nb, rrx, rry;
	float rr, rrr;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	if(pmap[y][x]==PT_GLOW)
	{
		sim->part_change_type(i, x, y, PT_PHOT);
	}
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_GLAS)
				{
					//fire_r[y/CELL][x/CELL] += rand()%200;	//D: Doesn't work with OpenGL, also shouldn't be here TODO: FIX THIS SHIT
					//fire_g[y/CELL][x/CELL] += rand()%200;
					//fire_b[y/CELL][x/CELL] += rand()%200;
					for (rrx=-1; rrx<=1; rrx++)
					{
						for (rry=-1; rry<=1; rry++)
						{
							if (x+rx+rrx>=0 && y+ry+rry>=0 && x+rx+rrx<XRES && y+ry+rry<YRES) {
								nb = sim->create_part(-1, x+rx+rrx, y+ry+rry, PT_BOMB);
								if (nb!=-1) {
									parts[nb].tmp = 1;
									parts[nb].life = 50;
									parts[nb].temp = 400.0f;
									parts[nb].vx = rand()%20-10;
									parts[nb].vy = rand()%20-10;
								}
							}
						}
					}
					sim->kill_part(i);
					return 1;
				}
				if ((r&0xFF)==PT_LCRY)
				{
					parts[r>>8].tmp2 = 5+rand()%5;
				}
				if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW || (r&0xFF)==PT_CBNW)
				{
					if(rand()%2)
					{
						sim->create_part(r>>8, x+rx, y+ry, PT_H2);
						sim->part_change_type(i, x, y, PT_O2);
						parts[i].life = 0;
						parts[i].ctype = 0;
						return 1;
					}
					else
					{
						sim->create_part(r>>8, x+rx, y+ry, PT_O2);
						sim->part_change_type(i, x, y, PT_H2);
						parts[i].life = 0;
						parts[i].ctype = 0;
						return 1;
					}
				}
				if ((r&0xFF)==PT_NEUT)
				{
					sim->part_change_type(r>>8, x+rx, y+ry, PT_H2);
					parts[r>>8].life = 0;
					parts[r>>8].ctype = 0;
				}
				if ((r&0xFF)==PT_DEUT)
				{
					if(parts[r>>8].life < 6000)
						parts[r>>8].life += 1;
					parts[r>>8].temp = 0;
					parts[i].temp = 0;
					sim->kill_part(i);
					return 1;
				}
				if ((r&0xFF)==PT_EXOT)
				{
					parts[r>>8].tmp2 += 5;
					parts[r>>8].life = 1000;
				}
				if ((sim->elements[r&0xFF].Properties & PROP_CONDUCTS) && ((r&0xFF)!=PT_NBLE||parts[i].temp<2273.15))
				{
					sim->create_part(-1, x+rx, y+ry, PT_SPRK);
					sim->kill_part(i);
					return 1;
				}
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_ELEC static int graphics(GRAPHICS_FUNC_ARGS)
int Element_ELEC::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_ELEC::~Element_ELEC() {}
