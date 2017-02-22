#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E186 PT_E186 186
Element_E186::Element_E186()
{
	Identifier = "DEFAULT_PT_E186";
	Name = "E186";
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

	Update = &Element_E186::update;
	Graphics = &Element_E186::graphics;
}

//#TPT-Directive ElementHeader Element_E186 static int update(UPDATE_FUNC_ARGS)
int Element_E186::update(UPDATE_FUNC_ARGS)
{
	int r, s, slife, sctype;
	float r2, r3;
	if (!(rand()%60))
	{
		sctype = parts[i].ctype;
		if (!sctype)
			s = sim->create_part(-3, x, y, PT_ELEC);
		else
			s = sim->create_part(-3, x, y, sctype);
		if(s >= 0)
		{
			parts[i].temp += 400.0f;
			parts[s].temp = parts[i].temp;
			sim->pv[y/CELL][x/CELL] += 1.5f;
			if (sctype == PT_GRVT)
				parts[s].tmp = 0;
		}
	}
	r = pmap[y][x];
	switch (r&0xFF)
	{
	case PT_PLSM:
		if (!(rand()%30))
		{
			sim->create_part(r>>8, x, y, PT_PLSM);
			parts[s].ctype = PT_NBLE;
		}
		break;
	case PT_O2:
		if (!(rand()%20))
		{
			sim->create_part(r>>8, x, y, PT_PLSM);
			s = sim->create_part(-3, x, y, PT_E186);
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
	case PT_EXOT:
		if (!(rand()%3))
		{
			sim->part_change_type(r>>8, x, y, PT_ISOZ);
			parts[r>>8].temp += 300;
		}
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
				s = r>>8;
				sim->create_part(s, x, y, PT_PHOT);
				r2 = (rand()%228+128)/127.0f;
				r3 = (rand()%360)*3.14159f/180.0f;
				parts[s].vx = r2*cosf(r3);
				parts[s].vy = r2*sinf(r3);
			}
			else
			{
				sim->create_part(r>>8, x, y, PT_E186);
			}
		}
		break;
	case PT_TUNG:
	case PT_BRMT:
		if ((r & 0xFF) == PT_TUNG && parts[r >> 8].ctype == PT_TUNG && !(rand()%50))
		{
			sim->part_change_type(r>>8, x, y, PT_E187);
		}
		break;
	default:
		break;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_E186 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E186::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_E186::~Element_E186() {}
