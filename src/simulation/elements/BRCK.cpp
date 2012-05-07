#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BRCK PT_BRCK 67
Element_BRCK::Element_BRCK()
{
    Identifier = "DEFAULT_PT_BRCK";
    Name = "BRCK";
    Colour = PIXPACK(0x808080);
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
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 251;
    Description = "Brick, breakable building material.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_HOT_GLOW;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = 8.8f;
    HighPressureTransition = PT_STNE;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 1223.0f;
    HighTemperatureTransition = PT_LAVA;
    
    Update = NULL;
    Graphics = NULL;
}

Element_BRCK::~Element_BRCK() {}