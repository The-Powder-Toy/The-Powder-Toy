#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PHOS PT_PHOS 20
Element_PHOS::Element_PHOS()
{
	Identifier = "DEFAULT_PT_PHOS";
	Name = "PHOS";
	Colour = PIXPACK(0x509647);
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
	
	Flammable = 50;
	Explosive = 0;
	Meltable = 5;
	Hardness = 0;
	
	Weight = 90;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 65;
	Description = "Phosphorus. Fertilizes plants and produces nitrogen.";
	
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
	
	Update = &Element_PHOS::update;
}

int Element_PHOS::update(UPDATE_FUNC_ARGS)
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
				if((r&0xFF)==PT_PLNT){
					parts[i].life++;
					parts[i].tmp++;
					parts[i].tmp2++;
					if(parts[i].life == 500){
						sim->CreateBox(x+2,y+2,x-2,y-2,PT_VINE,flags);
					}
					if(parts[i].tmp == 1000){
						sim->CreateBox(x+4,y+4,x-4,y-4,PT_VINE,flags);
					}
					if(parts[i].tmp2 == 1500){
						if (!(pmap[y-5][(int)parts[i].x]&0xFF)){
						sim->CreateBox(x+5,y+5,x-5,y-5,PT_N2,flags);
						parts[i].y = (parts[i].y-10);
						parts[r>>8].type = PT_VINE;
						parts[i].type=PT_WATR;
						}
						else{parts[i].type=PT_WATR;}
					}
				}
			}
		}
	return 0;
}

Element_PHOS::~Element_PHOS() {}
