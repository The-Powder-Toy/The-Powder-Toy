#include "simulation/Elements.h"

Element_CFBN::Element_CFBN()
{
	Identifier = "DEFAULT_PT_CFBN";
	Name = "CFBN";
	Colour = PIXPACK(0x7D5409);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
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
	Meltable = 5;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 65;
	Description = "Grown coffee bean.";
	
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
	
	Update = &Element_CFBN::update;
}


int Element_CFBN::update(UPDATE_FUNC_ARGS)
 {
    int r, rx, ry;
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
           if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					continue;
				{
				 if (((r&0xFF)==PT_WATR) && (parts[i].temp >= (R_TEMP + 100.0f)))
                {
                    sim->part_change_type(i,x,y,PT_COFE);
                }
				 if((r&0xFF)==PT_PLNT){
					parts[i].tmp2++;
					if(parts[i].tmp2 == 5000){
						sim->part_change_type(i,x,y,PT_COFF);
					}
				 }
			}
    }
    return 0;
 }

Element_CFBN::~Element_CFBN() {}
