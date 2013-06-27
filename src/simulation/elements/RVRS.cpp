#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RVRS PT_RVRS 14
Element_RVRS::Element_RVRS()
{
	Identifier = "DEFAULT_PT_RVRS";
	Name = "RVRS";
	Colour = PIXPACK(0xC3D2D4);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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
	Meltable = 1;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Reversium. Reverses some reactions.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_HOT_GLOW;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2000.0f;
	HighTemperatureTransition = PT_LRVS;
	
	Update = &Element_RVRS::update;;
	
}
//#TPT-Directive ElementHeader Element_RVRS static int update(UPDATE_FUNC_ARGS)
int Element_RVRS::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
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
					if((r&0xFF)==PT_URAN){
						parts[r>>8].type = PT_PLUT;
					}
					if((r&0xFF)==PT_BREC){
						parts[r>>8].type = PT_TUNG;
					}
					if((r&0xFF)==PT_STNE){
						parts[r>>8].type = PT_CLST;
					}
					if((r&0xFF)==PT_BRMT){
						parts[r>>8].type = PT_BMTL;
					}
					if((r&0xFF)==PT_BGLA){
						parts[r>>8].type = PT_SAND;
					}
					if((r&0xFF)==PT_GLAS){
						parts[r>>8].type = PT_SAND;
					}
					if((r&0xFF)==PT_DYST){
						parts[r>>8].type = PT_YEST;
					}
					if((r&0xFF)==PT_BANG){
						parts[r>>8].type = PT_NITR;
					}
					if((r&0xFF)==PT_DSTW){
						parts[r>>8].type = PT_WATR;
					}
					if((r&0xFF)==PT_FOG){
						parts[r>>8].type = PT_BOYL;
					}
					if((r&0xFF)==PT_ISOZ){
						parts[r>>8].type = PT_ACID;
					}	
					if((r&0xFF)==PT_FWRK){
						parts[r>>8].type = PT_DUST;
						parts[r>>8].tmp = 4;
					}
					if((r&0xFF)==PT_FIRW){
						parts[r>>8].type = PT_DUST;
						parts[r>>8].tmp = 4;
					}
					if(((r&0xFF)==PT_DUST) && (parts[r>>8].tmp != 4)){
						parts[r>>8].type = PT_GUNP;
					}
					if((r&0xFF)==PT_TRON){
						parts[r>>8].ctype = PT_TRON;
						parts[i].type = PT_REXP;
						parts[i].life = 0;
						parts[i].tmp = 9999999;
					}
					if((r&0xFF)==PT_FIRE){
						parts[r>>8].type = PT_CO2;
					}
					if(((r&0xFF)==PT_STKM) || ((r&0xFF)==PT_STKM2)){
						parts[r>>8].life = 1000;
					}
					if((r&0xFF)==PT_ACID){
						parts[r>>8].type = PT_H2;
						sim->create_part(i,x,y,PT_SALT);
						parts[i].type = PT_RVRS;
					}
					if(sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] >= 30.0f){
						parts[i].type = PT_WARP;
					}
					if(sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] <= -30.0f){
						parts[i].type = PT_WARP;
					}
				}
		}
	return 0;
}

Element_RVRS::~Element_RVRS() {}
