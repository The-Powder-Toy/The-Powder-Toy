#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ERAY PT_ERAY 170
Element_ERAY::Element_ERAY()
{
    Identifier = "DEFAULT_PT_ERAY";
    Name = "PRAY";
    Colour = PIXPACK(0xFFF400);
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
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 0;
    Description = "Property Ray. Will edit element values like a ray element would.";
    
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
    
    Update = &Element_CRAY::update;
    
}

//#TPT-Directive ElementHeader Element_ERAY static int update(UPDATE_FUNC_ARGS)
int Element_ERAY::update(UPDATE_FUNC_ARGS)
 {
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1, mode;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				// When sparked...
				if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
					
					for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
						if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
							break;
						}
						r = pmap[y+nyi+nyy][x+nxi+nxx];
						if (r) {
							if (parts[i].ctype == PT_BIZR) {
								// BIZR sets dcolour
								parts[i].ctype--;
							}
						}
					}
				}
			}
}

Element_CRAY::~Element_CRAY() {}