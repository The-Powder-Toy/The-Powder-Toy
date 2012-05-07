#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_145 PT_145 145
Element_145::Element_145()
{
    Identifier = "DEFAULT_PT_145";
    Name = "FROG";
    Colour = PIXPACK(0x00AA00);
    MenuVisible = 0;
    MenuSection = SC_LIFE;
    Enabled = 0;
    
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
    Hardness = 0;
    
    Weight = 100;
    
    Temperature = 9000.0f;
    HeatConduct = 40;
    Description = "Frogs S12/B34/3";
    
    State = ST_NONE;
    Properties = TYPE_SOLID|PROP_LIFE;
    
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

Element_145::~Element_145() {}