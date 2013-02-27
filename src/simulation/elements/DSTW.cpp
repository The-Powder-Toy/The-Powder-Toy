#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DSTW PT_DSTW 25
Element_DSTW::Element_DSTW()
{
    Identifier = "DEFAULT_PT_DSTW";
    Name = "DSTW";
    Colour = PIXPACK(0x1020C0);
    MenuVisible = 1;
    MenuSection = SC_LIQUID;
    Enabled = 1;
    
    Advection = 0.6f;
    AirDrag = 0.01f * CFDS;
    AirLoss = 0.98f;
    Loss = 0.95f;
    Collision = 0.0f;
    Gravity = 0.1f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 2;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 20;
    
    Weight = 30;
    
    Temperature = R_TEMP-2.0f	+273.15f;
    HeatConduct = 23;
    Description = "Distilled water, does not conduct electricity.";
    
    State = ST_LIQUID;
    Properties = TYPE_LIQUID|PROP_NEUTPENETRATE;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = 273.15f;
    LowTemperatureTransition = PT_ICEI;
    HighTemperature = 373.0f;
    HighTemperatureTransition = PT_WTRV;
    
    Update = &Element_DSTW::update;
    
}

//#TPT-Directive ElementHeader Element_DSTW static int update(UPDATE_FUNC_ARGS)
int Element_DSTW::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, rt;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				if (rt==PT_SALT && !(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
					// on average, convert 3 DSTW to SLTW before SALT turns into SLTW
					if (!rand()%3)
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
				if ((rt==PT_WATR||rt==PT_SLTW) && !(rand()%500))
				{
					sim->part_change_type(i,x,y,PT_WATR);
				}
				if (rt==PT_SLTW && !(rand()%10000))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
				}
				if ((rt==PT_RBDM||rt==PT_LRBD) && (sim->legacy_enable||parts[i].temp>12.0f) && !(rand()%500))
				{
					sim->part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				if (rt==PT_FIRE){
					sim->kill_part(r>>8);
					if(!(rand()%150)){
						sim->kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}


Element_DSTW::~Element_DSTW() {}
