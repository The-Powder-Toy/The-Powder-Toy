#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ALMN PT_ALMN 14
Element_ALMN::Element_ALMN()
{
	Identifier = "DEFAULT_PT_ALMN";
	Name = "ALMN";
	Colour = PIXPACK(0xC3D2D4);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Aluminum.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2000.0f;
	HighTemperatureTransition = PT_LAVA;
	
	Update = &Element_ALMN::update;;
	
}
//#TPT-Directive ElementHeader Element_RBTY static int update(UPDATE_FUNC_ARGS)
int Element_ALMN::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				{
					if(((r&0xFF)==PT_ELEC) || ((r&0xFF)==PT_PTRN)){
					parts[r>>8].type = PT_NTNO;
					}
					if((r&0xFF)==PT_BRAY){
						parts[r>>8].type = PT_PHOT;
					}
				}
		}
	return 0;
}
Element_ALMN::~Element_ALMN() {}