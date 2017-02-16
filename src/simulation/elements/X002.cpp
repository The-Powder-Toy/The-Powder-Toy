#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_X002 PT_X002 186
Element_X002::Element_X002()
{
	Identifier = "DEFAULT_PT_X002";
	Name = "X002";
	Colour = PIXPACK(0xDFEFFF);
	MenuVisible = 0;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	Temperature = R_TEMP+200.0f+273.15f;
	HeatConduct = 251;
	Description = "Experimental element.";

	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_RADIOACTIVE|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_X002::update;
	Graphics = &Element_X002::graphics;
}

//#TPT-Directive ElementHeader Element_X002 static int update(UPDATE_FUNC_ARGS)
int Element_X002::update(UPDATE_FUNC_ARGS)
{
	int r, s, slife;
	float r2, r3;
	if (!(rand()%60))
	{
		s = sim->create_part(-3, x, y, PT_ELEC);
		if(s >= 0)
		{
			parts[i].temp += 400.0f;
			parts[s].temp = parts[i].temp;
			sim->pv[y/CELL][x/CELL] += 1.5f;
		}
	}
	r = pmap[y][x];
	switch (r&0xFF)
	{
	case PT_O2:
		if (!(rand()%20))
		{
			sim->create_part(r>>8, x, y, PT_PLSM);
			s = sim->create_part(-3, x, y, PT_X002);
			slife = parts[i].life;
			if (slife)
				parts[s].life = slife + 30;
			else
				parts[s].life = 0;
		}
		break;
	case PT_FILT:
		sim->part_change_type(i, x, y, PT_PHOT);
		parts[i].ctype = 0x3FFFFFFF;
		break;
	case PT_ISOZ:
	case PT_ISZS:
		if (!(rand()%40))
		{
			slife = parts[i].life;
			if (slife)
				parts[i].life = slife + 50;
			else
				parts[i].life = 0;

			if (rand()%20)
			{
				sim->create_part(r>>8, x, y, PT_PHOT);
				r2 = (rand()%228+128)/127.0f;
				r3 = (rand()%360)*3.14159f/180.0f;
				parts[i].vx = rr*cosf(r2);
				parts[i].vy = rr*sinf(r3);
			}
			else
			{
				sim->create_part(r>>8, x, y, PT_X002);
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_X002 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_X002::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_X002::~Element_X002() {}
