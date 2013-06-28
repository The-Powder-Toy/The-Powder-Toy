#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_N2 PT_N2 203
Element_N2::Element_N2()
{
	Identifier = "DEFAULT_PT_N2";
	Name = "N2";
	Colour = PIXPACK(0xBFFFF8);
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
	Description = "Nitrogen.";
	
	State = ST_GAS;
	Properties = TYPE_GAS;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 194.65f;
	LowTemperatureTransition = PT_LNTG;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_N2::update;
	
}

//#TPT-Directive ElementHeader Element_N2 static int update(UPDATE_FUNC_ARGS)
int Element_N2::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	int flags;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r=sim->photons[y+ry][x+rx];
				if(!r)
					continue;
				{
					if((r&0xFF)==PT_GEL){
						parts[i].type = PT_NITR;
					}
					if((r&0xFF)==PT_PHOS){
						parts[i].type = PT_GUNP;
					}
					if((r&0xFF)==PT_PLNT){
						sim->CreateParts(x,y,rx,ry,PT_VINE,flags);
					}
				}
	}
	return 0;
}


Element_N2::~Element_N2() {}
