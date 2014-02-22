#include "simulation/Elements.h"

/*
  KSNS - Keyboard Sensor
  ====

  An element that sparks surrounding elements on a keypress - in two different 
  ways! Modes are set via TMP2. 

  Also it should be noted that this element does not work if key presses are 
  disabled via the Lua Script API. (change?)

  1. Simple Mode (TMP2 0)
  --------------

    Simple Mode, as you may expect, is very simple. It only works with arrow
    keys and sparks wires in the direction that an arrow key was pressed. You
    can then simply have a KSNS particle in the center and four wires leading
    out of it, which then are sparked as the keys are pressed.

  2. Advanced Mode (TMP2 1)
  ----------------

    This mode's terrible to use and is disabled by default, but contrarily 
    allows the save to capture mostly every key and shift-combination too. To 
    use this, you have to [1] set the element's TMP to which key code you want 
    to capture. Then the element will spark everywhere when that key has been 
    pressed. 

    Luckily, this element also sets its life to the last key code encountered, 
    so finding out the keycode of any combination is fairly simple. 
*/


//#TPT-Directive ElementClass Element_KSNS PT_KSNS 177
Element_KSNS::Element_KSNS()
{
  
  Identifier = "DEFAULT_PT_KSNS"; 
  Name = "KSNS"; 
  Colour = PIXPACK(0xFF0099);
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
  Description = "Keyboard Sensor. Shoots out electricity according to key presses.";
  
  State = ST_SOLID;
  Properties = TYPE_SOLID;

  Update = &Element_KSNS::update;
  Graphics = NULL; 

}


//#TPT-Directive ElementHeader Element_KSNS static int update(UPDATE_FUNC_ARGS)
int Element_KSNS::update(UPDATE_FUNC_ARGS)
{
  parts[i].life = Element_KSNS::key;

  // Now for the modes!
  int r, rx, ry, rt; 
 
  // basic mode, multiple arrows can be pressed at the same time
  if (parts[i].tmp == 0 && 
      Element_KSNS::arrows != 0) {
    
    // arrow up
    if (Element_KSNS::arrows & 0x01) {
      // walk two particles to the direction and turn into spark first thing
      r = pmap[y-1][x];
      // why can't i just short-circuited-OR the two
      if (!r)
        r = pmap[y-2][x];
      
      if (!r) 
        return 0; 
    
      rt = r&0xFF;

      if (sim->elements[rt].Properties & PROP_CONDUCTS &&
          parts[r>>8].life == 0) {
        parts[r>>8].life = 4;
        parts[r>>8].ctype = rt;
        sim->part_change_type(r>>8,parts[r>>8].x,parts[r>>8].y,PT_SPRK);
      }
    }

    // arrow left
    if (Element_KSNS::arrows & 0x02) {
      r = pmap[y][x-1];
      if (!r)
        r = pmap[y][x-2];
      if (!r) 
        return 0; 
    
      rt = r&0xFF;

      if (sim->elements[rt].Properties & PROP_CONDUCTS &&
          parts[r>>8].life == 0) {
        parts[r>>8].life = 4;
        parts[r>>8].ctype = rt;
        sim->part_change_type(r>>8,parts[r>>8].x,parts[r>>8].y,PT_SPRK);
      }
    }
    // arrow down
    if (Element_KSNS::arrows & 0x04) {
      r = pmap[y+1][x];
      if (!r)
        r = pmap[y+2][x];
      if (!r) 
        return 0; 
    
      rt = r&0xFF;

      if (sim->elements[rt].Properties & PROP_CONDUCTS &&
          parts[r>>8].life == 0) {
        parts[r>>8].life = 4;
        parts[r>>8].ctype = rt;
        sim->part_change_type(r>>8,parts[r>>8].x,parts[r>>8].y,PT_SPRK);
      }
    }
    // arrow right
    if (Element_KSNS::arrows & 0x08) {
      r = pmap[y][x+1];
      if (!r)
        r = pmap[y][x+2];
      if (!r) 
        return 0; 
      
      rt = r&0xFF;

      if (sim->elements[rt].Properties & PROP_CONDUCTS &&
          parts[r>>8].life == 0) {
        parts[r>>8].life = 4;
        parts[r>>8].ctype = rt;
        sim->part_change_type(r>>8,parts[r>>8].x,parts[r>>8].y,PT_SPRK);
      }
    }

    if (!r) 
      return 0; 
    
    rt = r&0xFF;

    if (sim->elements[rt].Properties & PROP_CONDUCTS &&
        parts[r>>8].life == 0) {
      parts[r>>8].life = 4;
      parts[r>>8].ctype = rt;
      sim->part_change_type(r>>8,parts[r>>8].x,parts[r>>8].y,PT_SPRK);
    }
  } 

  // advanced mode, one key at a time :C
  else if (parts[i].tmp2 == 1 &&
           Element_KSNS::key != 0 &&
           parts[i].tmp == Element_KSNS::key) {

    for (rx=-2; rx<3; rx++) {
      for (ry=-2; ry<3; ry++) {
        if (BOUNDS_CHECK && (rx || ry))
        {
          r = pmap[y+ry][x+rx];
          if (!r)
            continue;
          rt = r&0xFF; 

          if ((sim->elements[rt].Properties&PROP_CONDUCTS) && 
            !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && 
            parts[r>>8].life==0)
          {
            parts[r>>8].life = 4;
            parts[r>>8].ctype = rt;
            sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
          }
        }
      }
    }
  }

  return 0;
}

// Handles key press events sent from over there at GameController.cpp
// int key = the key code
// no modifiers and keychars available for now - too much logic would be involved
//#TPT-Directive ElementHeader Element_KSNS static int key
int Element_KSNS::key = 0; 

// up:    0b00000001 0x01, ~ 0b00001110 0x0e
// left:  0b00000010 0x02, ~ 0b00001101 0x0d
// down:  0b00000100 0x04, ~ 0b00001011 0x0b
// right: 0b00001000 0x08, ~ 0b00000111 0x07
//#TPT-Directive ElementHeader Element_KSNS static int arrows
int Element_KSNS::arrows = 0;

Element_KSNS::~Element_KSNS() {}

