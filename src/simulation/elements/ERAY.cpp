#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ERAY PT_ERAY 170
Element_ERAY::Element_ERAY()
{
    Identifier = "DEFAULT_PT_ERAY";
    Name = "ERAY";
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
    Properties = TYPE_SOLID;
    
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
					// this is too simple, scrapscrap
					for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
						if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
							break;
						}
						
						r = pmap[y+nyi+nyy][x+nxi+nxx];
						if (!r || (r&0xFF) == PT_BRAY) {
							int nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
							parts[nr].dcolour = 0x00022110;
							parts[nr].life = 5;
						}
						else if (r) {
							switch (parts[i].ctype) {
								// ctype dcolour flags life pavg temp tmp tmp2 type vx vy x y
							case PT_CLNE: // CLNE sets ctype.
								parts[r>>8].ctype = parts[i].tmp;
								break;
							case PT_BIZR: // BIZR sets dcolour
								parts[r>>8].dcolour = parts[i].dcolour;
								break;
							// Don't touch flags
							case PT_LIFE: // LIFE sets ... you've guessed it! life!
								parts[r>>8].life = parts[i].tmp;
								break;
							case PT_FIRE: // FIRE sets temp. 
								parts[r>>8].temp = parts[i].tmp % MAX_TEMP;
								break;
							case PT_ACEL: // ACEL sets tmp
								parts[r>>8].tmp = parts[i].tmp;
								break;
							case PT_DCEL: // DCEL sets tmp2
								parts[r>>8].tmp2 = parts[i].tmp;
								break;
							case PT_PSCN: // PSCN sets vx
								parts[r>>8].vx = parts[i].tmp;
								break;
							case PT_NSCN: // NSCN sets vy
								parts[r>>8].vy = parts[i].tmp;
								break;
							case PT_CONV: // CONV sets type. 
								parts[r>>8].type = parts[i].tmp % PT_NUM;
								break;
							default: // Anything else destroys the particle underneath.
								sim->kill_part(r>>8);
								break;
							}
							docontinue = 0;
						}
					}
				}
			}
	return 0;
}

Element_ERAY::~Element_ERAY() {}