#include "simulation/Elements.h"

/*
  ERAY  The Element Property Ray (PRAY)
  ====

  The Element that turns Children into Adults
  The Element that turns DUST into exploding SING
  The Element that turns your bomb into a feeble nothing
  The Element that makes your self-replicator work

  Powered by bit strings and a hundred-long Particle array in Simulation.cpp.

  HOW TO USE
  === == ===

  1. Write the fancy bit string for the element
     ( Add up the #defines you want from ERAY.h, 
       or run `bit.bor(elem.ERAY_TYPE_BIT, ...)` ) and set that as TMP.

  1.5 Select one of the 100 available slots for your copied element and 
      set the pixel's TMP2 to the slot - 1. (so 0, 1, ..., 98, 99 are valid)

  2. Spark it with PSCN to fire a beam to copy all properties of anything hit.
  3. Spark it with any other metal to fire a beam and paste the properties you 
     have set.

  * Unknown behavior when sparked with 'any other metal' before cloning.
  * Bit string defaults to ERAY_TYPE_BIT|ERAY_LIFE_BIT|ERAY_CTYPE_BIT
  
  WHAT IT DOES
  ==== == ====

  It's like portable PROP. 

  by boxmein I suppose 
*/

#include "ERAY.h"

//#TPT-Directive ElementClass Element_ERAY PT_ERAY 177
Element_ERAY::Element_ERAY()
{
  Identifier = "DEFAULT_PT_ERAY";
  Name = "ERAY";
  Colour = PIXPACK(0xFFEE33);
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
  Hardness = 1;
  
  Weight = 100;
  
  Temperature = R_TEMP+0.0f +273.15f;
  HeatConduct = 0;
  Description = "Element Ray. Sets properties on other elements.";
  
  State = ST_SOLID;
  Properties = TYPE_SOLID|PROP_LIFE_DEC;
  
  LowPressure = IPL;
  LowPressureTransition = NT;
  HighPressure = IPH;
  HighPressureTransition = NT;
  LowTemperature = ITL;
  LowTemperatureTransition = NT;
  HighTemperature = ITH;
  HighTemperatureTransition = NT;
  
  Update = &Element_ERAY::update;
  
}

//#TPT-Directive ElementHeader Element_ERAY static int update(UPDATE_FUNC_ARGS)
int Element_ERAY::update(UPDATE_FUNC_ARGS)
 {
  int q, r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1;

  // the usual
  for (rx=-1; rx<2; rx++)
    for (ry=-1; ry<2; ry++)
      if (BOUNDS_CHECK && (rx || ry))
      {
        r = pmap[y+ry][x+rx];
        if (!r)
          continue;

        if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {

          // draw a BRAY hint beam
          for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
            if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
              break;
            }
            q = pmap[y+nyi+nyy][x+nxi+nxx];
            if (!q) {
              int nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
              if (nr!=-1) {
                parts[nr].life = 7;
              }
            }
            else {
              // woo we hit something!

              // set own properties when sparked with PSCN
              // else set others' properties
              if (parts[r>>8].ctype==PT_PSCN) {

                // lol, invalid erays[] reference
                if (parts[i].tmp2 < 0) {
                  parts[i].tmp2 = 0;
                  continue;
                }
                else if (parts[i].tmp2 > 99) {
                  parts[i].tmp2 = 99;
                  continue;
                }

                // Copy over any property we care of
                sim->erays[ parts[i].tmp2 ].type    = parts[ q>>8 ].type;
                sim->erays[ parts[i].tmp2 ].life    = parts[ q>>8 ].life;
                sim->erays[ parts[i].tmp2 ].ctype   = parts[ q>>8 ].ctype;
                sim->erays[ parts[i].tmp2 ].x       = parts[ q>>8 ].x;
                sim->erays[ parts[i].tmp2 ].y       = parts[ q>>8 ].y;
                sim->erays[ parts[i].tmp2 ].vx      = parts[ q>>8 ].vx;
                sim->erays[ parts[i].tmp2 ].vy      = parts[ q>>8 ].vy;
                sim->erays[ parts[i].tmp2 ].temp    = parts[ q>>8 ].temp;
                sim->erays[ parts[i].tmp2 ].flags   = parts[ q>>8 ].flags;
                sim->erays[ parts[i].tmp2 ].tmp     = parts[ q>>8 ].tmp;
                sim->erays[ parts[i].tmp2 ].tmp2    = parts[ q>>8 ].tmp2;
                sim->erays[ parts[i].tmp2 ].dcolour = parts[ q>>8 ].dcolour;

                docontinue = 0;
                
              } else {
                // whoops what do I copy to
                if (parts[i].tmp==0) 
                  parts[i].tmp = ERAY_TYPE_BIT|ERAY_LIFE_BIT|ERAY_CTYPE_BIT;

                // lol, invalid erays[] reference
                if (parts[i].tmp2 < 0) {
                  parts[i].tmp2 = 0;
                  continue;
                }
                else if (parts[i].tmp2 > 99) {
                  parts[i].tmp2 = 99;
                  continue;
                }

                // give away what specified
                if (parts[i].tmp&ERAY_TYPE_BIT)
                  parts[q>>8].type = sim->erays[parts[i].tmp2].type;

                if (parts[i].tmp&ERAY_LIFE_BIT)
                  parts[q>>8].life = sim->erays[parts[i].tmp2].life;

                if (parts[i].tmp&ERAY_CTYPE_BIT)
                  parts[q>>8].ctype = sim->erays[parts[i].tmp2].ctype;

                if (parts[i].tmp&ERAY_X_BIT)
                  parts[q>>8].x = sim->erays[parts[i].tmp2].x;

                if (parts[i].tmp&ERAY_Y_BIT)
                  parts[q>>8].y = sim->erays[parts[i].tmp2].y;

                if (parts[i].tmp&ERAY_VX_BIT)
                  parts[q>>8].vx = sim->erays[parts[i].tmp2].vx;

                if (parts[i].tmp&ERAY_VY_BIT)
                  parts[q>>8].vy = sim->erays[parts[i].tmp2].vy;

                if (parts[i].tmp&ERAY_TEMP_BIT)
                  parts[q>>8].temp = sim->erays[parts[i].tmp2].temp;

                if (parts[i].tmp&ERAY_FLAGS_BIT)
                  parts[q>>8].flags = sim->erays[parts[i].tmp2].flags;

                if (parts[i].tmp&ERAY_TMP_BIT)
                  parts[q>>8].tmp = sim->erays[parts[i].tmp2].tmp;

                if (parts[i].tmp&ERAY_TMP2_BIT)
                  parts[q>>8].tmp2 = sim->erays[parts[i].tmp2].tmp2;

                if (parts[i].tmp&ERAY_DCOLOUR_BIT)
                  parts[q>>8].dcolour = sim->erays[parts[i].tmp2].dcolour;

                docontinue = 0;
              }
            }
          }


        }
      }
    //
  //
  return 0;
}


Element_ERAY::~Element_ERAY() {}
