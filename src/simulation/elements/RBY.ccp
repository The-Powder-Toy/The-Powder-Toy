#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RBTY PT_RBTY 173
Element_RBTY::Element_RBTY()
{
    Identifier = "DEFAULT_PT_RBTY";
    Name = "RBTY";
    Colour = PIXPACK(0x959615);
    MenuVisible = 1;
    MenuSection = SC_ELEC;
    Enabled = 1;
  
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.00f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f * CFDS;
    Falldown = 0;
  
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 20;
  
    Weight = 100;
  
    Temperature = R_TEMP+0.0f  +273.15f;
    HeatConduct = 251;
    Description = "Rechargeable battery.";
  
    State = ST_SOLID;
    Properties = TYPE_SOLID;
  
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 2273.0f;
    HighTemperatureTransition = PT_PLSM;
  
    Update = &Element_RBTY::update;
  
}
//#TPT-Directive ElementHeader Element_RBTY static int update(UPDATE_FUNC_ARGS)
int Element_RBTY::update(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    // Okay so just a looparound
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (BOUNDS_CHECK && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if (!r)
                    continue;
                if( sim->pv[x/CELL][y/CELL] >= 200.0f || parts[i].temp >= 373.15f)
                    sim->part_change_type(i, x, y, PT_PLEX);  
                if(parts[i].temp<=173.15)
                    parts[i].tmp=1;
                if((r&0xFF)==PT_SPRK)
                {
                    int t=parts[r>>8].ctype;
                    if(t==PT_PSCN)
                        parts[i].life++;
                }
                else if((r&0xFF)!=PT_PSCN && parts[i].life>0 && (sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life<=0)
                {
                    parts[r>>8].life = 4;
                    parts[r>>8].ctype = r&0xFF;
                    sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
                    parts[i].life--;
                }
            }
    return 0;
}
Element_RBTY::~Element_RBTY() {} 
