#include "simulation/Elements.h"
extern "C"
{
    #include "hmap.h"
}

//#TPT-Directive ElementClass Element_HFLM PT_HFLM 68
Element_HFLM::Element_HFLM()
{
    Identifier = "DEFAULT_PT_HFLM";
    Name = "CFLM";
    Colour = PIXPACK(0x8080FF);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;
    
    Advection = 0.9f;
    AirDrag = 0.04f * CFDS;
    AirLoss = 0.97f;
    Loss = 0.20f;
    Collision = 0.0f;
    Gravity = -0.1f;
    Diffusion = 0.00f;
    HotAir = 0.0005f	* CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 2;
    
    Temperature = 0.0f;
    HeatConduct = 88;
    Description = "Sub-zero flame.";
    
    State = ST_LIQUID;
    Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    Graphics = &Element_HFLM::graphics;
}

//#TPT-Directive ElementHeader Element_HFLM static int graphics(GRAPHICS_FUNC_ARGS)
int Element_HFLM::graphics(GRAPHICS_FUNC_ARGS)

{
	int caddress = restrict_flt(restrict_flt((float)((int)(cpart->life/2)), 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)hflm_data[caddress];
	*colg = (unsigned char)hflm_data[caddress+1];
	*colb = (unsigned char)hflm_data[caddress+2];
	
	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}


Element_HFLM::~Element_HFLM() {}