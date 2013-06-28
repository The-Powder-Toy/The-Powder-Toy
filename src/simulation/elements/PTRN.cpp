#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PTRN PT_PTRN 213
Element_PTRN::Element_PTRN()
{
	Identifier = "DEFAULT_PT_PTRN";
	Name = "PTRN";
	Colour = PIXPACK(0xE3E3E3);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = -1;
	
	Temperature = R_TEMP+200.0f+273.15f;
	HeatConduct = 251;
	Description = "Positron.";
	
	State = ST_GAS;
	Properties = TYPE_ENERGY|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_PTRN::update;
	Graphics = &Element_PTRN::graphics;
}

//#TPT-Directive ElementHeader Element_PTRN static int update(UPDATE_FUNC_ARGS)
int Element_PTRN::update(UPDATE_FUNC_ARGS)
 {
	int r, rt, rx, ry, nb, rrx, rry;
	int pp;
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
				if((r&0xFF)==PT_H2){
					parts[i].type = PT_PHOT;
					parts[r>>8].type = PT_PROT;
				}
				if(((r&0xFF)!=PT_NONE) && ((r&0xFF)!=PT_ELEC)){
					sim->create_part(i,x+2,y+2,PT_SPRK);
				}
				if(((r&0xFF)!=PT_NONE) && ((r&0xFF)!=PT_ELEC)){
					sim->create_part(i,x-2,y-2,PT_SPRK);
				}
				if((r&0xFF)==PT_SPRK){
					parts[i].type = PT_NONE;
				}
				}
			}
	return 0;
}

//#TPT-Directive ElementHeader Element_PTRN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PTRN::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_PTRN::~Element_PTRN() {}
