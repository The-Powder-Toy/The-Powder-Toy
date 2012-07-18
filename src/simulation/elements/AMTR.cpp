#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_AMTR PT_AMTR 72
Element_AMTR::Element_AMTR()
{
    Identifier = "DEFAULT_PT_AMTR";
    Name = "AMTR";
    Colour = PIXPACK(0x808080);
    MenuVisible = 1;
    MenuSection = SC_NUCLEAR;
    Enabled = 1;
    
    Advection = 0.7f;
    AirDrag = 0.02f * CFDS;
    AirLoss = 0.96f;
    Loss = 0.80f;
    Collision = 0.00f;
    Gravity = 0.10f;
    Diffusion = 1.00f;
    HotAir = 0.0000f * CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 70;
    Description = "Anti-Matter, Destroys a majority of particles";
    
    State = ST_NONE;
    Properties = TYPE_PART;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_AMTR::update;
    
}

//#TPT-Directive ElementHeader Element_AMTR static int update(UPDATE_FUNC_ARGS)
int Element_AMTR::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_AMTR && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_NONE && (r&0xFF)!=PT_PHOT && (r&0xFF)!=PT_VOID && (r&0xFF)!=PT_BHOL && (r&0xFF)!=PT_NBHL && (r&0xFF)!=PT_PRTI && (r&0xFF)!=PT_PRTO)
				{
					parts[i].life++;
					if (parts[i].life==4)
					{
						sim->kill_part(i);
						return 1;
					}
					if (10>(rand()/(RAND_MAX/100)))
						sim->create_part(r>>8, x+rx, y+ry, PT_PHOT);
					else
						sim->kill_part(r>>8);
					sim->pv[y/CELL][x/CELL] -= 2.0f;
				}
			}
	return 0;
}


Element_AMTR::~Element_AMTR() {}