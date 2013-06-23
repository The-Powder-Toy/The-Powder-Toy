#include "simulation/Elements.h"

Element_ALPH::Element_ALPH()
{
	Identifier = "DEFAULT_PT_ALPH";
	Name = "ALPH";
	Colour = PIXPACK(0xFFDD00);
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
	Description = "Alpha particle.";
	
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
	
	Update = &Element_ALPH::update;
	Graphics = &Element_ALPH::graphics;
}


int Element_ALPH::update(UPDATE_FUNC_ARGS)
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
				if((r&0xFF)==PT_URAN){
					sim->parts[r>>8].type = PT_PLUT;
				}
				if((r&0xFF)==PT_PLUT){
					sim->create_part(i,x,y,PT_NEUT);
				}
				if((r&0xFF)!=PT_NONE){
					sim->create_part(i,x+2,y+2,PT_SPRK);
				}
				if((r&0xFF)!=PT_NONE){
					sim->create_part(i,x-2,y-2,PT_SPRK);
				}
				if((r&0xFF)==PT_SPRK){
					parts[i].type = PT_NONE;
				}
				if(((r&0xFF)==PT_ELEC) && (parts[i].temp < (MAX_TEMP/2))){
					parts[i].type = PT_HELI;
					parts[r>>8].type = PT_NONE;
				}
			}
		}
	return 0;
}


int Element_ALPH::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_ALPH::~Element_ALPH() {}
