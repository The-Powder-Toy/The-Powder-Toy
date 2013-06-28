#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_COFE PT_COFE 183
Element_COFE::Element_COFE()
{
	Identifier = "DEFAULT_PT_COFE";
	Name = "COFF";
	Colour = PIXPACK(0xB07B19);
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
	HeatConduct = 29;
	Description = "Coffee. Heals stickmen.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPASS;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_COFE::update;
	
}
//#TPT-Directive ElementHeader Element_COFE static int update(UPDATE_FUNC_ARGS)
int Element_COFE::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_STKM) || ((r&0xFF)==PT_STKM2))
				{
					parts[r>>8].life++;
					parts[i].type = PT_NONE;
				}
			}
	return 0;
}

Element_COFE::~Element_COFE() {}
