#include "simulation/Elements.h"
#ifdef WIN
#include "windows.h"
bool win = true;
#else
bool win = false;
sudo apt-get install beep
#endif
//#TPT-Directive ElementClass Element_BEEP PT_BEEP 180
Element_BEEP::Element_BEEP()
{
        Identifier = "DEFAULT_PT_BEEP";
        Name = "BEEP";
        Colour = PIXPACK(0x40A060);
        MenuVisible = 1;
        MenuSection = SC_ELEC;
        Enabled = 1;
       
        Advection = 0.0f;
        AirDrag = 0.00f * CFDS;
        AirLoss = 0.90f;
        Loss = 0.00f;
        Collision = 0.0f;
        Gravity = 0.0f;
        Diffusion = 0.00f;
        HotAir = 0.000f * CFDS;
        Falldown = 0;
       
        Flammable = 0;
        Explosive = 0;
        Meltable = 0;
        Hardness = 2;
       
        Weight = 100;
       
        Temperature = R_TEMP+0.0f       +273.15f;
        HeatConduct = 0;
        Description = "Beep generator. Made by cyberdragon";
       
        State = ST_SOLID;
        Properties = TYPE_SOLID;
       
        LowPressure = IPL;
        LowPressureTransition = NT;
        HighPressure = 15.0f;
        HighPressureTransition = PT_BRMT;
        LowTemperature = ITL;
        LowTemperatureTransition = NT;
        HighTemperature = ITH;
        HighTemperatureTransition = NT;
       
        Update = &Element_BEEP::update;
}
 
//#TPT-Directive ElementHeader Element_BEEP static int update(UPDATE_FUNC_ARGS)
int Element_BEEP::update(UPDATE_FUNC_ARGS)
{
        int r, rx, ry, rt;
                for (rx=-1; rx<2; rx++)
                        for (ry=-1; ry<2; ry++)
                                if (BOUNDS_CHECK)
                                {
                                        r = sim->photons[y+ry][x+rx];
                                        if (!r)
                                                r = pmap[y+ry][x+rx];
                                        if (!r)
                                                continue;
                                        if((r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_PSCN && parts[r>>8].life==3)
                                        {
                                                if(win == true)
                                                {
                                                Beep(parts[i].tmp, parts[i].tmp2);
                                                break;
                                                }
                                                else
                                                {
                                                #ifdef WIN
                                                        win = true;
                                                        return 0;
                                                #else
                                                try:
                                                        import winsound
                                                        except ImportError:
                                                        import os
                                                         def playsound(frequency,duration):
                                                                 #apt-get install beep
                                                                os.system('beep -f %s -l %s' % (frequency,duration))
                                                                else:
                                                                 def playsound(frequency,duration):
                                                                  winsound.Beep(parts[i].tmp,parts[i].tmp2)
                                                #endif
                                                }
                                        }
                                }
                                        return 0;
}
 
 
Element_BEEP::~Element_BEEP() {}