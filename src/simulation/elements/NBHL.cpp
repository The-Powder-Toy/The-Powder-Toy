#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NBHL PT_NBHL 150
Element_NBHL::Element_NBHL()
{
	Identifier = "DEFAULT_PT_NBHL";
	Name = "BHOL";
	Colour = PIXPACK(0x202020);
	MenuVisible = 0;
	MenuSection = SC_SPECIAL;
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
	Meltable = 0;
	Hardness = 0;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "Black hole, sucks in particles using gravity. (Requires Newtonian gravity)";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_NBHL::update;
	
}

//#TPT-Directive ElementHeader Element_NBHL static int update(UPDATE_FUNC_ARGS)
int Element_NBHL::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
                pmap[y+ry][x+rx] &&
                (pmap[y+ry][x+rx]&0xFF)!=PT_SDST&&
                (pmap[y+ry][x+rx]&0xFF)!=0xFF)
    {
        r = pmap[y+ry][x+rx];
        if(r&0xFF){
			sim->pv[x/CELL][y/CELL] = 256;
			sim->part_change_type(i,x,y,PT_STAR);
    }
    return 0;
 }
}


Element_NBHL::~Element_NBHL() {}
