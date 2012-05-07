#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_147 PT_147 147
Element_147::Element_147()
{
    Identifier = "DEFAULT_PT_147";
    Name = "WIND";
    Colour = PIXPACK(0x101010);
    MenuVisible = 0;
    MenuSection = SC_SPECIAL;
    Enabled = 0;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.90f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f  * CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 100;
    
    Temperature = 0.0f;
    HeatConduct = 40;
    Description = "";
    
    State = ST_NONE;
    Properties = ST_NONE;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = NULL;
    Graphics = NULL;
}

Element_147::~Element_147() {}