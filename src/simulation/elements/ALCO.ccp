#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ALCO PT_ALCO 195
Element_ALCO::Element_ALCO()
{
    Identifier = "DEFAULT_PT_ALCO";
    Name = "ALCO";
    Colour = PIXPACK(0xD7E8F7);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;

    Advection = 0.6f;
    AirDrag = 0.01f * CFDS;
    AirLoss = 0.97f;
    Loss = 0.96f;
    Collision = 0.0f;
    Gravity = 0.9f;
    Diffusion = 0.00f;
    HotAir = 0.000f  * CFDS;
    Falldown = 2;

    Flammable = 200;
    Explosive = 195;
    Meltable = 0;
    Hardness = 10;

    Weight = 49;

    Temperature = R_TEMP+0.0  +273.15f;
    HeatConduct = 250;
    Description = "Alcohol. Flammable. Evaporates at low temps. Sterile.";
 
    State = ST_LIQUID;
    Properties = TYPE_LIQUID|PROP_DEADLY|PROP_NEUTABSORB;

    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = 273.15f;
    LowTemperatureTransition = PT_ICEI;
    HighTemperature = 373.0f;
    HighTemperatureTransition = PT_WTRV;

    Update = &Element_ALCO::update;

}

//#TPT-Directive ElementHeader Element_ALCO static int update(UPDATE_FUNC_ARGS)

 int Element_ALCO::update(UPDATE_FUNC_ARGS)
{
    int r, rx, ry, rt;

    if(!(rand()%200))
    {
        parts[i].tmp2 = rand()%40;
    }
    else if(parts[i].tmp2!=20)
    {
        parts[i].tmp2 -= (parts[i].tmp2>20)?1:-1;
    }

    for (rx=-1; rx<2; rx++)
        for (ry=-1; ry<2; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if (!r)
                    continue;
                rt = parts[r>>8].type;
                if (parts[i].temp >= 315.15f+273.15f && rt!=PT_FIRE && rt!=PT_LAVA && rt!=PT_PLSM)
                    {sim->part_change_type(i, x, y, PT_GAS); }
                    parts[i].ctype = PT_ALCO;
            }
    return 0;
}



//#TPT-Directive ElementHeader Element_ALCO static int graphics(GRAPHICS_FUNC_ARGS)

Element_ALCO::~Element_ALCO() {}
