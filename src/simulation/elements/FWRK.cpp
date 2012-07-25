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
		int r = (rand()%245+11);
		int g = (rand()%245+11);
		int b = (rand()%245+11);
		int n;
		float angle, magnitude;
		unsigned col = (r<<16) | (g<<8) | b;
		for (n=0; n<40; n++)
		{
			np = sim->create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = ((rand()%60)+40)*0.05f;
				angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy + sinf(angle)*magnitude - 2.5f;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = rand()%40+70;
				parts[np].temp = (rand()%500)+5750.0f;
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		sim->pv[y/CELL][x/CELL] += 8.0f;
		sim->kill_part(i);
		return 1;
	}
	return 0;
}


Element_FWRK::~Element_FWRK() {}