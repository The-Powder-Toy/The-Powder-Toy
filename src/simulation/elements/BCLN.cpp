#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BCLN PT_BCLN 93
Element_BCLN::Element_BCLN()
{
    Identifier = "DEFAULT_PT_BCLN";
    Name = "BCLN";
    Colour = PIXPACK(0xFFD040);
    MenuVisible = 1;
    MenuSection = SC_SPECIAL;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.97f;
    Loss = 0.50f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 12;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 251;
    Description = "Breakable Clone.";
    
    State = ST_NONE;
    Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_BCLN::update;
    
}

//#TPT-Directive ElementHeader Element_BCLN static int update(UPDATE_FUNC_ARGS)
int Element_BCLN::update(UPDATE_FUNC_ARGS)
 {
	if (!parts[i].life && sim->pv[y/CELL][x/CELL]>4.0f)
		parts[i].life = rand()%40+80;
	if (parts[i].life)
	{
		float advection = 0.1f;
		parts[i].vx += advection*sim->vx[y/CELL][x/CELL];
		parts[i].vy += advection*sim->vy[y/CELL][x/CELL];
	}
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOLALT)))
	{
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN &&
				        (r&0xFF)!=PT_BCLN && (r&0xFF)!=PT_STKM &&
				        (r&0xFF)!=PT_PBCN && (r&0xFF)!=PT_STKM2 &&
				        (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						if ((r&0xFF)==PT_LIFE || (r&0xFF)==PT_LAVA)
							parts[i].tmp = parts[r>>8].ctype;
					}
				}
	}
	else {
		if (parts[i].ctype==PT_LIFE) sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype|(parts[i].tmp<<8));
		else if (parts[i].ctype!=PT_LIGH || (rand()%30)==0)
		{
			int np = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
			if (np>=0)
			{
				if (parts[i].ctype==PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && sim->elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA)
					parts[np].ctype = parts[i].tmp;
			}
		}
	}
	return 0;
}


Element_BCLN::~Element_BCLN() {}
