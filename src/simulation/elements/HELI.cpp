#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_HELI PT_HELI 80
Element_HELI::Element_HELI()
{
	Identifier = "DEFAULT_PT_HELI";
	Name = "HELI";
	Colour = PIXPACK(0xCB62F5);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;
	
	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.75f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	
	Weight = 1;
	
	Temperature = R_TEMP+215.15;
	HeatConduct = 88;
	Description = "Helium.";
	
	State = ST_GAS;
	Properties = TYPE_GAS;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_HELI::update;
	
}

//#TPT-Directive ElementHeader Element_HELI static int update(UPDATE_FUNC_ARGS)
int Element_HELI::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	int flags;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if((r&0xFF)==PT_NEUT){
						parts[i].type = PT_HLM3;
						parts[r>>8].type = PT_NONE;
					}
					if(parts[i].temp >= (MAX_TEMP/2)){
						sim->create_part(i,x,y,PT_ELEC);
					}
				}
	}
	return 0;
}


Element_HELI::~Element_HELI() {}
