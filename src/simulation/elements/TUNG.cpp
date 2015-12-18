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

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3695.0f;// TUNG melts in its update function instead of in the normal way, but store the threshold here so that it can be changed from Lua
	HighTemperatureTransition = NT;

	Update = &Element_TUNG::update;
	Graphics = &Element_TUNG::graphics;
}

//#TPT-Directive ElementHeader Element_TUNG static int update(UPDATE_FUNC_ARGS)
int Element_TUNG::update(UPDATE_FUNC_ARGS)
{
	bool splode = false;
	const float MELTING_POINT = sim->elements[PT_TUNG].HighTemperature;

	if(parts[i].temp > 2400.0)
	{
		int r, rx, ry;
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
			parts[i].temp = restrict_flt(MELTING_POINT + (rand()%600) + 200, MIN_TEMP, MAX_TEMP);
		}
		parts[i].vx += (rand()%100)-50;
		parts[i].vy += (rand()%100)-50;
		return 1;
	}
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y/CELL][x/CELL];
	float diff = parts[i].pavg[1] - parts[i].pavg[0];
	if (diff > 0.50f || diff < -0.50f)
	{
		sim->part_change_type(i,x,y,PT_BRMT);
		parts[i].ctype = PT_TUNG;
		return 1;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_TUNG static int graphics(GRAPHICS_FUNC_ARGS)
int Element_TUNG::graphics(GRAPHICS_FUNC_ARGS)
{
	const float MELTING_POINT = ren->sim->elements[PT_TUNG].HighTemperature;
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
