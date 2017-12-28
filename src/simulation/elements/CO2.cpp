#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CO2 PT_CO2 80
Element_CO2::Element_CO2()
{
	Identifier = "DEFAULT_PT_CO2";
	Name = "CO2";
	Colour = PIXPACK(0x666666);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 1.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 88;
	Description = "Carbon Dioxide. Heavy gas, drifts downwards. Carbonates water and turns to dry ice when cold.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 194.65f;
	LowTemperatureTransition = PT_DRIC;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_CO2::update;
}

//#TPT-Directive ElementHeader Element_CO2 static int update(UPDATE_FUNC_ARGS)
int Element_CO2::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (parts[i].ctype==5 && !(rand()%2000))
					{
						if (sim->create_part(-1, x+rx, y+ry, PT_WATR)>=0)
							parts[i].ctype = 0;
					}
					continue;
				}
				if (TYP(r)==PT_FIRE){
					sim->kill_part(ID(r));
					if(!(rand()%30)){
						sim->kill_part(i);
						return 1;
					}
				}
				else if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && !(rand()%50))
				{
					sim->part_change_type(ID(r), x+rx, y+ry, PT_CBNW);
					if (parts[i].ctype==5) //conserve number of water particles - ctype=5 means this CO2 hasn't released the water particle from BUBW yet
					{
						sim->create_part(i, x, y, PT_WATR);
						return 0;
					}
					else
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
	if (parts[i].temp > 9773.15 && sim->pv[y/CELL][x/CELL] > 200.0f)
	{
		if (!(rand()%5))
		{
			int j;
			sim->create_part(i,x,y,PT_O2);
			j = sim->create_part(-3,x,y,PT_NEUT);
			if (j != -1)
				parts[j].temp = MAX_TEMP;
			if (!(rand()%50))
			{
				j = sim->create_part(-3,x,y,PT_ELEC);
				if (j != -1)
					parts[j].temp = MAX_TEMP;
			}
			parts[i].temp = MAX_TEMP;
			sim->pv[y/CELL][x/CELL] += 100;
		}
	}
	return 0;
}


Element_CO2::~Element_CO2() {}
