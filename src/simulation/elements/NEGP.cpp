#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NEGP PT_NEGP 170
Element_NEGP::Element_NEGP()
{
Identifier = "DEFAULT_PT_NEGP";
Name = "NEGP";
Colour = PIXPACK(0x0000FF);
MenuVisible = 1;
MenuSection = SC_GAS;
Enabled = 1;

Advection = 0.9f;
AirDrag = 0.04f * CFDS;
AirLoss = 0.97f;
Loss = 0.20f;
Collision = 0.0f;
Gravity = -0.1f;
Diffusion = 0.30f;
HotAir = 0.001f * CFDS;
Falldown = 0;

Flammable = 0;
Explosive = 0;
Meltable = 0;
Hardness = 0;

Weight = 1;

Temperature = R_TEMP 10000.0f +273.15f;
HeatConduct = 5;
Description = "Negative plasma.";

State = ST_GAS;
Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

LowPressure = IPL;
LowPressureTransition = NT;
HighPressure = IPH;
HighPressureTransition = NT;
LowTemperature = ITL;
LowTemperatureTransition = NT;
HighTemperature = ITH;
HighTemperatureTransition = NT;

Update = NULL;
Graphics = &Element_NEGP::graphics;
}
//#TPT-Directive ElementHeader Element_NEGP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_NEGP::graphics(GRAPHICS_FUNC_ARGS)
{
int caddress = restrict_flt(restrict_flt((float)cpart->life, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
*colr = (unsigned char)ren->nplasma_data[caddress];
*colg = (unsigned char)ren->nplasma_data[caddress+1];
*colb = (unsigned char)ren->nplasma_data[caddress+2];

*firea = 255;
*firer = *colr;
*fireg = *colg;
*fireb = *colb;

*pixel_mode = PMODE_GLOW | PMODE_ADD; //Clear default, don't draw pixel
*pixel_mode |= FIRE_ADD;
//Returning 0 means dynamic, do not cache
return 0;
}
Element_NEGP::~Element_NEGP() {}