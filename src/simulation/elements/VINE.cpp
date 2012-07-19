#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VINE PT_VINE 114
Element_VINE::Element_VINE()
{
    Identifier = "DEFAULT_PT_VINE";
    Name = "VINE";
    Colour = PIXPACK(0x079A00);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.95f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 20;
    Explosive = 0;
    Meltable = 0;
    Hardness = 10;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 65;
    Description = "Vine, grows";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 573.0f;
    HighTemperatureTransition = PT_FIRE;
    
    Update = &Element_VINE::update;
    
}

//#TPT-Directive ElementHeader Element_VINE static int update(UPDATE_FUNC_ARGS)
int Element_VINE::update(UPDATE_FUNC_ARGS)
 {
	int r, np, rx =(rand()%3)-1, ry=(rand()%3)-1;
	if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if (1>rand()%15)
			sim->part_change_type(i,x,y,PT_PLNT);
		else if (!r)
		{
			np = sim->create_part(-1,x+rx,y+ry,PT_VINE);
			if (np<0) return 0;
			parts[np].temp = parts[i].temp;
			sim->part_change_type(i,x,y,PT_PLNT);
		}
	}
	return 0;
}


Element_VINE::~Element_VINE() {}