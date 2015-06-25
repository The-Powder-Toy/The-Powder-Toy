#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FREN PT_FREN 179
Element_FREN::Element_FREN()
{
    Identifier = "DEFAULT_PT_FREN";
    Name = "FREN";
    Colour = PIXPACK(0x89AE51);
    MenuVisible = 1;
    MenuSection = SC_GAS;
    Enabled = 1;
    
    Advection = 1.0f;
    AirDrag = 0.01f * CFDS;
    AirLoss = 0.99f;
    Loss = 0.30f;
    Collision = -0.1f;
    Gravity = 0.0f;
    Diffusion = 1.75f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 3;
    Explosive = 0;
    Meltable = 0;
    Hardness = 3;
    
    Weight = 1;
    
    Temperature = R_TEMP+53.f+273.15f;
    HeatConduct = 200;
    Description = "Freon gas. Releases heat corresponding to its tmp and changes to liquid under pressure.";
    
    State = ST_GAS;
    Properties = TYPE_GAS;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_FREN::update;
    Graphics = &Element_FREN::graphics;
}

//#TPT-Directive ElementHeader Element_FREN static int update(UPDATE_FUNC_ARGS)
int Element_FREN::update(UPDATE_FUNC_ARGS)
{
    return 0;
}

//#TPT-Directive ElementHeader Element_FREN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FREN::graphics(GRAPHICS_FUNC_ARGS)
{
	return 1;
}

Element_FREN::~Element_FREN() {}
