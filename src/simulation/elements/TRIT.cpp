#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_TRIT PT_TRIT 220
Element_TRIT::Element_TRIT()
{
	Identifier = "DEFAULT_PT_TRIT";
	Name = "TRIT";
	Colour = PIXPACK(0x072AB5);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;
	
	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = -0.1f;
	Diffusion = 0.75f;
	HotAir = 0.0003f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 4;
	
	Weight = 1;
	
	Temperature = R_TEMP+0.0f+273.15f;
	HeatConduct = 48;
	Description = "Tritium. Isotope of hydrogen.";
	
	State = ST_GAS;
	Properties = TYPE_GAS;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 371.0f;
	LowTemperatureTransition = ST;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_TRIT::update;
	
}

//#TPT-Directive ElementHeader Element_TRIT static int update(UPDATE_FUNC_ARGS)
int Element_TRIT::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if((r&0xFF)==PT_NEUT){
					parts[i].type = PT_ALPH;
					parts[r>>8].type = PT_NONE;
				}
				if((r&0xFF)==PT_O2){
					parts[i].ctype = PT_O2;
				}
				if(((r&0xFF)==PT_FIRE) && (parts[i].ctype == PT_O2)){
					parts[i].type = PT_3H2O;
					parts[r>>8].type = PT_NONE;
				}
				if(((r&0xFF)==PT_DTRM) && (sim->pv[x/CELL][y/CELL] >= 200.0f)){
					parts[i].type = PT_HELI;
					parts[r>>8].type = PT_NEUT;
					parts[i].temp = MAX_TEMP;
					parts[r>>8].temp = MAX_TEMP;
				}
			}
	return 0;
}


Element_TRIT::~Element_TRIT() {}
