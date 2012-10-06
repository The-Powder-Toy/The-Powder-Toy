#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VIBR PT_VIBR 
Element_VIBR::Element_VIBR()
{
    Identifier = "DEFAULT_PT_VIBR";
    Name = "VIBR";
    Colour = PIXPACK(0x002900);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.85f;
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
    HeatConduct = 251;
    Description = "Vibranium. Stores energy and releases it in violent explosions.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_LIFE_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_VIBR::update;
    Graphics = &Element_VIBR::graphics;
}

int Element_VIBR::update(UPDATE_FUNC_ARGS) {
int r, rx, ry, rt, lol, ftw, trade;
	rt = parts[r>>8].type;
	//Heat absorption code
	if (parts[i].temp>(273.15f+0.0f))
		{
			parts[i].ctype+=3;
			parts[i].temp-=3;
		}
	if (parts[i].temp<(273.15f-1.0f))
		{
			parts[i].ctype-=3;
			parts[i].temp+=3;
		}
	//Pressure absorption code
	if (sim->pv[y/CELL][x/CELL]>0 && sim->pv[y/CELL][x/CELL]>0.5)
		{
			parts[i].tmp++;
			sim->pv[y/CELL][x/CELL]--;
		}
	if (sim->pv[y/CELL][x/CELL]<-5)
		{
			sim->pv[y/CELL][x/CELL]++;
		}
	//Release sparks before explode
	if (parts[i].ctype>3200)
		{
			parts[r>>8].life = 4;
			parts[r>>8].ctype = rt;
			sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
		}
	//Release all heat
	if ((3000<parts[i].ctype && parts[i].ctype<3333) || parts[i].tmp>90 || (80<parts[i].tmp2 && parts[i].tmp2<100))
		{
			for(rx=-1; rx<2; rx++)
					for(ry=-1; ry<2; ry++)
						if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && pmap[y+ry][x+rx] && (pmap[y+ry][x+rx]&0xFF)!=PT_VIBR&& (pmap[y+ry][x+rx]&0xFF)!=0xFF)
						{
							r = pmap[y+ry][x+rx];
							if(parts[r>>8].temp+ (parts[r>>8].temp*0.2f)<=MAX_TEMP)
							{
								parts[r>>8].temp += parts[r>>8].temp*0.2f;
							}
							else 
							{
								parts[r>>8].temp = MAX_TEMP;
							}
						}
						if (3200<parts[i].ctype || parts[i].tmp>95 || 90<parts[i].tmp2)
						parts[i].ctype-=3;
		}
	//Explosion code
	if (parts[i].ctype>3333 || parts[i].tmp>100 || parts[i].tmp2>100)
		{
			sim->part_change_type(i,x,y,PT_EXOT);
			sim->create_part(-1,x,y-1,PT_ELEC);
			sim->create_part(-1,x,y-1,PT_NEUT);
			sim->create_part(-1,x,y-1,PT_PHOT);
			sim->create_part(-1,x,y-1,PT_BREC);
			parts[i].temp=9000;
			sim->pv[y/CELL][x/CELL]=256;
		}
	//Neighbor check loop
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				//Melts into EXOT
				if ((r&0xFF)==PT_EXOT && 1>(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_EXOT);				
				}
				//Absorbs energy particles
				if ((r&0xFF)==PT_NEUT || (r&0xFF)==PT_ELEC || (r&0xFF)==PT_PHOT)
				{
					parts[i].tmp2+=3;
					sim->kill_part(r>>8);					
				}
				if ((r&0xFF)==PT_BOYL)
				{
					sim->part_change_type(i,x,y,PT_BVBR);					
				}
			}
	for ( trade = 0; trade<9; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_VIBR&&(parts[i].tmp>parts[r>>8].tmp)&&parts[i].tmp>0)//diffusion
			{
				lol = parts[i].tmp - parts[r>>8].tmp;
				if (lol ==1)
				{
					parts[r>>8].tmp ++;
					parts[i].tmp --;
					trade = 9;
				}
				else if (lol>0)
				{
					parts[r>>8].tmp += lol/2;
					parts[i].tmp -= lol/2;
					trade = 9;
				}
			}
			if ((r&0xFF)==PT_VIBR&&(parts[i].tmp2>parts[r>>8].tmp2)&&parts[i].tmp2>0)//diffusion
			{
				ftw = parts[i].tmp2 - parts[r>>8].tmp2;
				if (ftw ==1)
				{
					parts[r>>8].tmp2++;
					parts[i].tmp2 --;
					trade = 9;
				}
				else if (ftw>0)
				{
					parts[r>>8].tmp2 += ftw/2;
					parts[i].tmp2 -= ftw/2;
					trade = 9;
				}
			}
		}
	}	
	return 0;
}

int Element_VIBR::graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = fmax(cpart->tmp,cpart->temp);
	if (cpart->ctype > 1000 && cpart->ctype < 2000)
	{
		*colr += (int)restrict_flt((cpart->ctype-1000)/20,0,51);
		*colg += (int)restrict_flt((cpart->ctype-1000)/18,0,55);
		*colb += (int)restrict_flt((cpart->ctype-1000)/20,0,51);
	}
	else if (cpart->ctype >= 2000 && cpart->ctype < 3000)
	{
		*colr += (int)restrict_flt((cpart->ctype-2000)/20+51,51,100);
		*colg += (int)restrict_flt((cpart->ctype-2000)/31+55,55,87);
		*colb += (int)restrict_flt((cpart->ctype-2000)/20+51,51,100);
	}
	else if (cpart->ctype >= 3000 && cpart->ctype < 3200)
	{
		*colr += (int)restrict_flt((cpart->ctype-3000)/1.7+100,100,218);
		*colg += (int)restrict_flt((cpart->ctype-3000)/2+87,87,192);
		*colb += (int)restrict_flt((cpart->ctype-3000)/1.7+100,100,218);
	}
	else if (cpart->ctype >= 3200)
	{
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_ADD;
		*firea = 90;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*colr = 146;
		*colg = 158;
		*colb = 113;
	}
	
	return 0;
}

Element_VIBR::~Element_VIBR() {}