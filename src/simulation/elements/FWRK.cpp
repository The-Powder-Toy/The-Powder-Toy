#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FWRK PT_FWRK 98
Element_FWRK::Element_FWRK()
{
    Identifier = "DEFAULT_PT_FWRK";
    Name = "FWRK";
    Colour = PIXPACK(0x666666);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;
    
    Advection = 0.4f;
    AirDrag = 0.01f * CFDS;
    AirLoss = 0.99f;
    Loss = 0.95f;
    Collision = 0.0f;
    Gravity = 0.4f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 97;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 100;
    Description = "First fireworks made, activated by heat/neutrons.";
    
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
    
    Update = &Element_FWRK::update;
    Graphics = NULL;
}

//#TPT-Directive ElementHeader Element_FWRK static int update(UPDATE_FUNC_ARGS)
int Element_FWRK::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, np;
	if ((parts[i].temp>400&&(9+parts[i].temp/40)>rand()%100000&&parts[i].life==0&&!pmap[y-1][x])||parts[i].ctype==PT_DUST)
	{
		np = sim->create_part(-1, x , y-1 , PT_FWRK);
		if (np!=-1)
		{
			parts[np].vy = rand()%8-22;
			parts[np].vx = rand()%20-rand()%20;
			parts[np].life=rand()%15+25;
			parts[np].dcolour = parts[i].dcolour;
			sim->kill_part(i);
			return 1;
		}
	}
	if (parts[i].life>=45)
		parts[i].life=0;
	if ((parts[i].life<3&&parts[i].life>0)||(parts[i].vy>6&&parts[i].life>0))
	{
		int q = (rand()%255+1);
		int w = (rand()%255+1);
		int e = (rand()%255+1);
		for (rx=-1; rx<2; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					if (5>=rand()%8)
					{
						if (!pmap[y+ry][x+rx])
						{
							np = sim->create_part(-1, x+rx, y+ry , PT_DUST);
							sim->pv[y/CELL][x/CELL] += 2.00f*CFDS;
							if (np!=-1)
							{
								parts[np].vy = -(rand()%10-1);
								parts[np].vx = ((rand()%2)*2-1)*rand()%(5+5)+(parts[i].vx)*2 ;
								parts[np].life= rand()%37+18;
								parts[np].tmp=q;
								parts[np].tmp2=w;
								parts[np].ctype=e;
								parts[np].temp= rand()%20+6000;
								parts[np].dcolour = parts[i].dcolour;
							}
						}
					}
				}
		sim->kill_part(i);
		return 1;
	}
	return 0;
}


Element_FWRK::~Element_FWRK() {}