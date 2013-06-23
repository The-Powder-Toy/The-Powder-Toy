#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_HLM3 PT_HLM3 80
Element_HLM3::Element_HLM3()
{
	Identifier = "DEFAULT_PT_HLM3";
	Name = "HLM3";
	Colour = PIXPACK(0xAE20E6);
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
	Description = "Helium-3. Radioactive helium isotope.";
	
	State = ST_GAS;
	Properties = TYPE_GAS|PROP_RADIOACTIVE;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_HLM3::update;
	
}

//#TPT-Directive ElementHeader Element_HLM3 static int update(UPDATE_FUNC_ARGS)
int Element_HLM3::update(UPDATE_FUNC_ARGS)
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
					if(((r&0xFF)==PT_DTRM) && (sim->pv[x/CELL][y/CELL] >= 200.0f)){
					parts[i].type = PT_HELI;
					parts[r>>8].type = PT_PROT;
					parts[i].temp = MAX_TEMP;
					parts[r>>8].temp = MAX_TEMP;
				}
					if(parts[i].temp >= (MAX_TEMP/2)){
						sim->create_part(i,x,y,PT_ALPH);
					}
				}
	}
	return 0;
}


Element_HLM3::~Element_HLM3() {}
