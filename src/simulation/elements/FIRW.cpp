#include "simulation/Elements.h"
extern "C"
{
	#include "hmap.h"
}
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
	if (parts[i].tmp<=0) {
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
						parts[i].life = rand()%40+60;
					}
				}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life<=0) {
			parts[i].tmp=2;
		} else {
			// TODO: different gravity modes + Newtonian gravity
			parts[i].vy = -parts[i].life*0.04f - 0.1f;
		}
	}
	else if (parts[i].tmp>=2)
	{
		float angle, magnitude;
		int caddress = (rand()%200)*3;
		int n;
		unsigned col = (((unsigned char)(firw_data[caddress]))<<16) | (((unsigned char)(firw_data[caddress+1]))<<8) | ((unsigned char)(firw_data[caddress+2]));
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


//#TPT-Directive ElementHeader Element_FIRW static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FIRW::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->tmp > 0)
	{
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}


Element_FIRW::~Element_FIRW() {}