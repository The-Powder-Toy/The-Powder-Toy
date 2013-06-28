#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_COFF PT_COFF 184
Element_COFF::Element_COFF()
{
	Identifier = "DEFAULT_PT_COFF";
	Name = "CFSD";
	Colour = PIXPACK(0xD0FF6B);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;
	
	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 65;
	Description = "Plantable coffee bean. Grows with plant and water.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_NEUTPENETRATE;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;
	
	Update = &Element_COFF::update;
}
//#TPT-Directive ElementHeader Element_COFF static int update(UPDATE_FUNC_ARGS)
int Element_COFF::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, np;
	int flags;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					continue;
				{
				if((r&0xFF)==PT_WATR){
					parts[r>>8].type = PT_VINE;
				}
				if((r&0xFF)==PT_PLNT){
					if (!(pmap[y-1][(int)parts[i].x]&0xFF)) {
						parts[i].y -= 2;
						sim->create_part(r>>8,x+rx,y+ry,PT_STALK);
					}
					parts[i].life++;
					if(parts[i].life == 500){
						sim->CreateBox(x+2,y+2,x-2,y-2,PT_CFBN,flags);
						parts[i].type = PT_STALK;
					}
				}
			}
		}
	return 0;
}

Element_COFF::~Element_COFF() {}
