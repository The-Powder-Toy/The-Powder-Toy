#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ANAR PT_ANAR 113
Element_ANAR::Element_ANAR()
{
	Identifier = "DEFAULT_PT_ANAR";
	Name = "ANAR";
	Colour = PIXPACK(0xFFFFEE);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = -0.7f;
	AirDrag = -0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.1f;
	Gravity = -0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 85;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Anti-air. Very light dust, which behaves opposite gravity.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_ANAR::update;
}

//#TPT-Directive ElementHeader Element_ANAR static int update(UPDATE_FUNC_ARGS)
int Element_ANAR::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	   
	//if (parts[i].temp >= 0.23)
	// parts[i].temp --;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_CFLM && !(rand()%4))
				{
					sim->part_change_type(i,x,y,PT_CFLM);
					parts[i].life = rand()%150+50;
					parts[r>>8].temp = parts[i].temp = 0;
					sim->pv[y/CELL][x/CELL] -= 0.5;
				}
			}
	return 0;
}


Element_ANAR::~Element_ANAR() {}
