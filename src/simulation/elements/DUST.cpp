#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DUST PT_DUST 1
Element_DUST::Element_DUST()
{
    Identifier = "DEFAULT_PT_DUST";
    Name = "DUST";
    Colour = PIXPACK(0xFFE0A0);
    MenuVisible = 1;
    MenuSection = SC_POWDERS;
    Enabled = 1;
    
    Advection = 0.7f;
    AirDrag = 0.02f * CFDS;
    AirLoss = 0.96f;
    Loss = 0.80f;
    Collision = 0.0f;
    Gravity = 0.1f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 1;
    
    Flammable = 10;
    Explosive = 0;
    Meltable = 0;
    Hardness = 30;
    
    Weight = 85;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 70;
    Description = "Very light dust. Flammable.";
    
    State = ST_SOLID;
    Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    Graphics = &Element_DUST::graphics;
}

//#TPT-Directive ElementHeader Element_DUST static int graphics(GRAPHICS_FUNC_ARGS)
int Element_DUST::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->life >= 1)
	{
		*firea = 120;
		*firer = *colr = cpart->flags;
		*fireg = *colg = cpart->tmp;
		*fireb = *colb = cpart->ctype;
		if (ren->decorations_enable && cpart->dcolour)
		{
			int a = (cpart->dcolour>>24)&0xFF;
			*firer = *colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
			*fireg = *colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
			*fireb = *colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
		}
		*pixel_mode |= PMODE_GLOW | FIRE_ADD;
		/**firea = 255;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;*/
	}
	return 0;
}


Element_DUST::~Element_DUST() {}