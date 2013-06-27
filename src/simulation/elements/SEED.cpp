#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SEED PT_SEED 20
Element_SEED::Element_SEED()
{
	Identifier = "DEFAULT_PT_SEED";
	Name = "SEED";
	Colour = PIXPACK(0xA17A0E);
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
	Description = "Plantable tree seed.";
	
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
	
	Update = &Element_SEED::update;
}
//#TPT-Directive ElementHeader Element_SEED static int update(UPDATE_FUNC_ARGS)
int Element_SEED::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, np;
	int flags;
	int q=0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					continue;
				{
					
				if((((r&0xFF)==PT_PLNT) && (parts[i].life < 100)) || (((r&0xFF)==PT_WOOD) && (parts[i].life < 100))){
					if (!((pmap[y-1][(int)parts[i].x]&0xFF)==PT_PLNT)) {
					parts[i].y -= 2;
					sim->CreateBox(x+2,y+2,x-2,y-2,PT_WOOD,flags);
					parts[i].life++;
					for(int p=0;p<100;p++){
					for(parts[i].tmp=0;parts[i].tmp<5;parts[i].tmp++)
					{q=parts[i].tmp;}}
						if(parts[i].life==100){
							sim->CreateLine(x+20+q,y+6+q,x,y,PT_WOOD);
							sim->CreateLine(x-20+q,y+6+q,x,y,PT_WOOD);
							sim->CreateLine(x+30+q,y-10+q,x,y,PT_WOOD);
							sim->CreateLine(x-30+q,y-10+q,x,y,PT_WOOD);
							sim->CreateLine(x+30+q,y-30+q,x,y,PT_WOOD);
							sim->CreateLine(x-30+q,y-30+q,x,y,PT_WOOD);
							sim->CreateLine(x+15+q,y-40+q,x,y,PT_WOOD);
							sim->CreateLine(x-15+q,y-40+q,x,y,PT_WOOD);
							sim->CreateLine(x,y-54+q,x,y,PT_WOOD);

							sim->CreateLine(x+1,y-60,x+1,y,PT_VINE);
							sim->CreateLine(x-1,y-60,x-1,y,PT_VINE);
							sim->CreateLine(x-16,y-40,x,y,PT_VINE);
							sim->CreateLine(x+16,y-40,x,y,PT_VINE);

							sim->CreateLine(x+21+q,y+7+q,x,y,PT_WOOD);
							sim->CreateLine(x-21+q,y+7+q,x,y,PT_WOOD);
							sim->CreateLine(x+31+q,y-11+q,x,y,PT_WOOD);
							sim->CreateLine(x-31+q,y-11+q,x,y,PT_WOOD);
							sim->CreateLine(x+31+q,y-31+q,x,y,PT_WOOD);
							sim->CreateLine(x-31+q,y-31+q,x,y,PT_WOOD);
							sim->CreateLine(x+16+q,y-41+q,x,y,PT_WOOD);
							sim->CreateLine(x-16+q,y-41+q,x,y,PT_WOOD);

							sim->CreateBox(x+10,y+10,x-10,y-10,PT_VINE,flags);
						}
					}
				}
			}
			if((r&0xFF)==PT_WOOD){
				parts[r>>8].ctype = PT_SEED;
			}
		}
	return 0;
}

Element_SEED::~Element_SEED() {}
