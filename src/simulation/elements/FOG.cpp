#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FOG PT_FOG 92
Element_FOG::Element_FOG()
{
	Identifier = "DEFAULT_PT_FOG";
	Name = "FOG";
	Colour = PIXPACK(0xAAAAAA);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
	Enabled = 1;
	
	Advection = 0.8f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.4f;
	Loss = 0.70f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.99f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;
	
	Weight = 1;
	
	Temperature = 243.15f;
	HeatConduct = 100;
	Description = "Not quite Steam";
	
	State = ST_GAS;
	Properties = TYPE_GAS|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 373.15f;
	HighTemperatureTransition = PT_WTRV;
	
	Update = &Element_FOG::update;
	
}

//#TPT-Directive ElementHeader Element_FOG static int update(UPDATE_FUNC_ARGS)
int Element_FOG::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->elements[r&0xFF].State==ST_SOLID && !(rand()%10) && parts[i].life==0 && !((r&0xFF)==PT_CLNE || (r&0xFF)==PT_PCLN)) // TODO: should this also exclude BCLN?
				{
					sim->part_change_type(i,x,y,PT_RIME);
				}
				if ((r&0xFF)==PT_SPRK)
				{
					parts[i].life += rand()%20;
				}
			}
	return 0;
}


Element_FOG::~Element_FOG() {}
