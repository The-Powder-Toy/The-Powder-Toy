#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BARI PT_BARI 14
Element_BARI::Element_BARI()
{
	Identifier = "DEFAULT_PT_BARI";
	Name = "BARI";
	Colour = PIXPACK(0xE0D91B);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 1;
	Explosive = 1;
	Meltable = 0;
	Hardness = 20;
	
	Weight = 35;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Barium. Used in fireworks.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_BARI::update;;
	
}
//#TPT-Directive ElementHeader Element_BARI static int update(UPDATE_FUNC_ARGS)
int Element_BARI::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
	int flags;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				{
					if(((r&0xFF)==PT_FIRE) || ((r&0xFF)==PT_SDST) || ((r&0xFF)==PT_LAVA) || ((r&0xFF)==PT_PLSM) || ((r&0xFF)==PT_LIGH) || ((r&0xFF)==PT_THDR)){
						sim->create_part(i,x,y,PT_FIRE);
						parts[i].life = 100;
						for(int c=0; c<50; c++){
							parts[i].life--;
							if(parts[i].life == 0){
								parts[i].type = PT_SMKE;
							}
						}
					}
					if((r&0xFF)==PT_O2){
						sim->create_part(i,x,y,PT_FIRE);
					}
					if((r&0xFF)==PT_PSTE){
						parts[i].life++;
						if(parts[i].life == 250){
							parts[i].type = PT_FIRW;
							parts[r>>8].type = PT_FWRK;
						}
					}
				}
		}
	return 0;
}

Element_BARI::~Element_BARI() {}