#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_TUNG PT_TUNG 171
Element_TUNG::Element_TUNG()
{
	Identifier = "DEFAULT_PT_TUNG";
	Name = "TUNG";
	Colour = PIXPACK(0x505050);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Tungsten. Brittle metal with a very high melting point.";

	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	/*HighTemperature = 3895.0f;
	HighTemperatureTransition = PT_LAVA;*/

	Update = &Element_TUNG::update;
	Graphics = &Element_TUNG::graphics;
	
}

#define MELTING_POINT	3695.0

//#TPT-Directive ElementHeader Element_TUNG static int update(UPDATE_FUNC_ARGS)
int Element_TUNG::update(UPDATE_FUNC_ARGS)
{
	bool splode = false;
	if(parts[i].temp > 2400.0)
	{
		int r, rx, ry, rt;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if((r&0xFF) == PT_O2)
					{
						splode = true;
					}
				}
	}
	if((parts[i].temp > MELTING_POINT && !(rand()%20)) || splode)
	{
		if(!(rand()%50))
		{
			sim->pv[y/CELL][x/CELL] += 50.0f;
		}
		else if(!(rand()%100))
		{
			sim->part_change_type(i, x, y, PT_FIRE);
			parts[i].life = rand()%500;
			return 1;
		}
		else
		{
			sim->part_change_type(i, x, y, PT_LAVA);
			parts[i].ctype = PT_TUNG;
			return 1;
		}
		if(splode)
		{
			parts[i].temp = MELTING_POINT + (rand()%600) + 200;
		}
		parts[i].vx += (rand()%100)-50;
		parts[i].vy += (rand()%100)-50;
		return 1;
	} 
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y/CELL][x/CELL];
	if (parts[i].pavg[1]-parts[i].pavg[0] > 0.50f || parts[i].pavg[1]-parts[i].pavg[0] < -0.50f)
	{
		sim->part_change_type(i,x,y,PT_BRMT);
		parts[i].ctype = PT_TUNG;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_TUNG static int graphics(GRAPHICS_FUNC_ARGS)
int Element_TUNG::graphics(GRAPHICS_FUNC_ARGS)
{
	double startTemp = (MELTING_POINT - 1500.0);
	double tempOver = (((cpart->temp - startTemp)/1500.0)*M_PI) - (M_PI/2.0);
	if(tempOver > -(M_PI/2.0))
	{
		if(tempOver > (M_PI/2.0))
			tempOver = (M_PI/2.0);
		double gradv = sin(tempOver) + 1.0;
		*firer = (int)(gradv * 258.0);
		*fireg = (int)(gradv * 156.0);
		*fireb = (int)(gradv * 112.0);
		*firea = 30;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}

Element_TUNG::~Element_TUNG() {}
