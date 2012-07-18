#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WOOD PT_WOOD 17
Element_WOOD::Element_WOOD()
{
    Identifier = "DEFAULT_PT_WOOD";
    Name = "WOOD";
    Colour = PIXPACK(0xC0A040);
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
    
    Flammable = 20;
    Explosive = 0;
    Meltable = 0;
    Hardness = 15;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 164;
    Description = "Solid. Flammable.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID | PROP_NEUTPENETRATE;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 873.0f;
    HighTemperatureTransition = PT_FIRE;
    
    Update = NULL;
    
}

Element_WOOD::~Element_WOOD() {}