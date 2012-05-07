#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_INVIS PT_INVIS 115
Element_INVIS::Element_INVIS()
{
    Identifier = "DEFAULT_PT_INVIS";
    Name = "INVS";
    Colour = PIXPACK(0x00CCCC);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
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
    Meltable = 0;
    Hardness = 15;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 164;
    Description = "Invisible to everything while under pressure.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID | PROP_NEUTPASS;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    Graphics = &Element_INVIS::graphics;
}

//#TPT-Directive ElementHeader Element_INVIS static int graphics(GRAPHICS_FUNC_ARGS)
int Element_INVIS::graphics(GRAPHICS_FUNC_ARGS)

{
	//pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f
	if(cpart->tmp)
	{
		*cola = 100;
		*colr = 15;
		*colg = 0;
		*colb = 150;
		*pixel_mode &= PMODE;
		*pixel_mode |= PMODE_BLEND;
	} 
	return 0;
}


Element_INVIS::~Element_INVIS() {}