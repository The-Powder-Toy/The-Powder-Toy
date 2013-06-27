#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SDST PT_SDST 201
Element_SDST::Element_SDST()
{
	Identifier = "DEFAULT_PT_SDST";
	Name = "SDST";
	Colour = PIXPACK(0xD2F59D);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = MAX_TEMP;
	HeatConduct = 251;
	Description = "Star dust.";
	
	State = ST_SOLID;
	Properties = TYPE_PART|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_SDST::update;
	Graphics = &Element_SDST::graphics;
}

//#TPT-Directive ElementHeader Element_SDST static int update(UPDATE_FUNC_ARGS)
int Element_SDST::update(UPDATE_FUNC_ARGS)
 {
    int r, rx, ry;
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
			parts[i].temp+=(parts[i].temp);
		 if((r&0xFF)==PT_STAR){
			parts[i].type=PT_NONE;
		 }
    return 0;
 }

//#TPT-Directive ElementHeader Element_SDST static int graphics(GRAPHICS_FUNC_ARGS)
int Element_SDST::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_SDST::~Element_SDST() {}
