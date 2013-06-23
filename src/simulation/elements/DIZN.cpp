#include "simulation/Elements.h"

Element_DIZN::Element_DIZN()
{
	Identifier = "DEFAULT_PT_DIZN";
	Name = "DIZN";
	Colour = PIXPACK(0x808F22);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 80;
	Explosive = 0;
	Meltable = 5;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f +275.0f;
	HeatConduct = 251;
	Description = "Diazane jet fuel.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_DIZN::update;
}


int Element_DIZN::update(UPDATE_FUNC_ARGS)
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
				if((r&0xFF)==PT_BARI){
					sim->create_part(i,x,y,PT_PLEX);
				}
				if(parts[i].temp>=parts[r>>8].temp){
					parts[i].vy++;
				}
			}
    }
    return 0;
 }

Element_DIZN::~Element_DIZN() {}
