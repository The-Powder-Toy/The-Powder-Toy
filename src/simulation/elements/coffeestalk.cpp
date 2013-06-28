#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_STALK PT_STALK 186
Element_STALK::Element_STALK()
{
	Identifier = "DEFAULT_PT_STALK";
	Name = "STALK";
	Colour = PIXPACK(0xC0A040);
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 65;
	Description = "coffee plant stalk";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;
	
	Update = &Element_STALK::update;
	
}

//#TPT-Directive ElementHeader Element_STALK static int update(UPDATE_FUNC_ARGS)
int Element_STALK::update(UPDATE_FUNC_ARGS)
{
	int r, np, rx =(rand()%3)-1, ry=(rand()%3)-1;
	if (BOUNDS_CHECK && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if (!(rand()%15))
			sim->part_change_type(i,x,y,PT_PLNT);
		else if (!r)
		{
			np = sim->create_part(-1,x+rx,y+ry,PT_STALK);
			if (np<0) return 0;
			parts[np].temp = parts[i].temp;
			parts[i].tmp = 1;
			sim->part_change_type(i,x,y,PT_PLNT);
		}
	}
	if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

Element_STALK::~Element_STALK() {}
