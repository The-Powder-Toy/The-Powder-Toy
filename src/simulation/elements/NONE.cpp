#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NONE PT_NONE 0
Element_NONE::Element_NONE()
{
    Identifier = "DEFAULT_PT_NONE";
    Name = "";
    Colour = PIXPACK(0x000000);
    MenuVisible = 1;
    MenuSection = SC_SPECIAL;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 1.00f;
    Loss = 0.00f;
    Collision = 0.0f;
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
    HeatConduct = 251;
    Description = "Erases particles.";
    
    State = ST_NONE;
    Properties = 0;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    
}

Element_NONE::~Element_NONE() {}