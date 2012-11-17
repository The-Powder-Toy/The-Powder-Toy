#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_THRM PT_THRM 65
Element_THRM::Element_THRM()
{
    Identifier = "DEFAULT_PT_THRM";
    Name = "THRM";
    Colour = PIXPACK(0xA08090);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;
    
    Advection = 0.4f;
    AirDrag = 0.04f * CFDS;
    AirLoss = 0.94f;
    Loss = 0.95f;
    Collision = -0.1f;
    Gravity = 0.3f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 2;
    Hardness = 2;
    
    Weight = 90;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 211;
    Description = "Thermite. Burns at extremely high temperature.";
    
    State = ST_SOLID;
    Properties = TYPE_PART;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_THRM::update;
    
}

//#TPT-Directive ElementHeader Element_THRM static int update(UPDATE_FUNC_ARGS)
int Element_THRM::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM || (r&0xFF)==PT_LAVA)) // TODO: could this go in update_PYRO?
				{
					if (1>(rand()%500)) {
						sim->part_change_type(i,x,y,PT_LAVA);
						parts[i].ctype = PT_BMTL;
						parts[i].temp = 3500.0f;
						sim->pv[y/CELL][x/CELL] += 50.0f;
					} else {
						sim->part_change_type(i,x,y,PT_LAVA);
						parts[i].life = 400;
						parts[i].ctype = PT_THRM;
						parts[i].temp = 3500.0f;
						parts[i].tmp = 20;
					}
				}
			}
	return 0;
}


Element_THRM::~Element_THRM() {}