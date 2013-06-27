#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BOX PT_BOX 14
Element_BOX::Element_BOX()
{
	Identifier = "DEFAULT_PT_BOX";
	Name = "BOX";
	Colour = PIXPACK(0x969696);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 0;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;
	
	Flammable = 100;
	Meltable = 1;
	Hardness = 1;
	
	Weight = 99;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Falling solid box. WIP (Does not 'fall' anymore :(";
	
	State = ST_SOLID;
	Properties = TYPE_PART;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2000.0f;
	HighTemperatureTransition = PT_LRVS;
	
	Update = &Element_BOX::update;;
	
}
//#TPT-Directive ElementHeader Element_BOX static int RuleTable[9][9]
int Element_BOX::RuleTable[9][9] =
{
	{1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1},
};
//#TPT-Directive ElementHeader Element_LOLZ static int lolz[XRES/9][YRES/9];
int Element_BOX::box[XRES/9][YRES/9];
//#TPT-Directive ElementHeader Element_BOX static int update(UPDATE_FUNC_ARGS)
int Element_BOX::update(UPDATE_FUNC_ARGS)
  {
	int r, rt, rx, ry, nb, rrx, rry;
	int obj;
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
					if ((!(pmap[y-1][(int)parts[i].x]&0xFF))&&(pmap[y-1][(int)parts[i].x]&0xFF)==PT_BOX) {
						parts[i].type = PT_NONE;
						sim->create_part(r>>8,r+rx,r+ry+2,PT_BOX);
					}
				}
		}
	return 0;
}

Element_BOX::~Element_BOX() {}
