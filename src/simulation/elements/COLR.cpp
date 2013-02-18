#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_COLR PT_COLR 170
Element_COLR::Element_COLR()
{
    Identifier = "DEFAULT_PT_COLR";
    Name = "COLR";
    Colour = PIXPACK(0x000056);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.90f;
    Loss = 0.00f;
    Collision = 0.00f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 0;
    Description = "Color. Changes color based on tmp value.";
    
    State = ST_SOLID;
    Properties = TYPE_PART | PROP_HOT_GLOW;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    Graphics = &Element_COLR::graphics;
}
//#TPT-Directive ElementHeader Element_COLR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_COLR::graphics(GRAPHICS_FUNC_ARGS)

{
	int x, temp_bin = (int)((cpart->temp-273.0f)*0.025f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	cpart->ctype = 0x1F << temp_bin;
	*colg = 0;
	*colb = 0;
	*colr = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*cola = 127;
	*colr *= x;
	*colg *= x;
	*colb *= x;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND;
	return 0;
}
Element_COLR::~Element_COLR() {}