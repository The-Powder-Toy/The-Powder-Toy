#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_IRON PT_IRON 76
Element_IRON::Element_IRON()
{
    Identifier = "DEFAULT_PT_IRON";
    Name = "IRON";
    Colour = PIXPACK(0x707070);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
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
    Hardness = 50;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 251;
    Description = "Rusts with salt, can be used for electrolysis of WATR";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 1687.0f;
    HighTemperatureTransition = PT_LAVA;
    
    Update = &Element_IRON::update;
    
}

//#TPT-Directive ElementHeader Element_IRON static int update(UPDATE_FUNC_ARGS)
int Element_IRON::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((((r&0xFF) == PT_SALT && 15>(rand()/(RAND_MAX/700))) ||
				        ((r&0xFF) == PT_SLTW && 30>(rand()/(RAND_MAX/2000))) ||
				        ((r&0xFF) == PT_WATR && 5 >(rand()/(RAND_MAX/6000))) ||
				        ((r&0xFF) == PT_O2   && 2 >(rand()/(RAND_MAX/500))) ||
				        ((r&0xFF) == PT_LO2))&&
				        (!(parts[i].life))
				   )
				{
					sim->part_change_type(i,x,y,PT_BMTL);
					parts[i].tmp=(rand()/(RAND_MAX/10))+20;
				}
			}
	return 0;
}


Element_IRON::~Element_IRON() {}