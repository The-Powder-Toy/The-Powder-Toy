#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E187 PT_E187 187
Element_E187::Element_E187()
{
	Identifier = "DEFAULT_PT_E187";
	Name = "E187";
	Colour = PIXPACK(0xFF60D0);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 24;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Experimental element. acts like ISOZ.";

	Properties = TYPE_LIQUID | PROP_LIFE_DEC | PROP_NOSLOWDOWN | PROP_TRANSPARENT;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 160.0f;
	LowTemperatureTransition = PT_E188;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E187::update;
	Graphics = &Element_E187::graphics;
}

//#TPT-Directive ElementHeader Element_E187 static int update(UPDATE_FUNC_ARGS)
int Element_E187::update(UPDATE_FUNC_ARGS)
{ // for both ISZS and ISOZ
	int r, rx, ry, stmp;
	switch (parts[i].ctype) {
	case 0:
		if (!parts[i].life)
		{
			stmp = parts[i].tmp;
			if (!(rand()%10000) && !(stmp & 1))
			{
				Element_E187::createPhotons(sim, i, x, y, stmp, parts);
			}
			r = sim->photons[y][x];
			if ((r & 0xFF) == PT_PHOT && !(rand()%100))
			{
				Element_E187::createPhotons(sim, i, x, y, stmp, parts);
			}
		}
		break;
	case 1:
		if (parts[i].tmp)
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
					if (BOUNDS_CHECK)
					{
						r = pmap[y+ry][x+rx];
						if ((r & 0xFF) == PT_E187 && !parts[r>>8].ctype && !(rand()%200))
						{
							parts[r>>8].tmp &= 0xFFFFFFFE;
							sim->pv[y/CELL][x/CELL] += 3.0f;
						}
					}
		break;
	default:
		break;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_E187 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E187::graphics(GRAPHICS_FUNC_ARGS)
{
	switch(cpart->ctype) {
	case 1:
		*colr = 0xFF;
		*colg = 0x80;
		*colb = 0xBB;
		break;
	case 0:
	default:
		break;
	}

	return 0;
}

//#TPT-Directive ElementHeader Element_E187 static int createPhotons(Simulation* sim, int i, int x, int y, int tmp, Particle *parts)
int Element_E187::createPhotons(Simulation* sim, int i, int x, int y, int tmp, Particle *parts)
{
	int r, s;
	float r2, r3;
	const int cooldown = 15;
	s = sim->create_part(-3, x, y, PT_PHOT);
	if(s >= 0) {
		r2 = (rand()%128+128)/127.0f;
		r3 = (rand()%360)*3.1415926f/180.0f;
		parts[s].vx = r2*cosf(r3);
		parts[s].vy = r2*sinf(r3);
		parts[i].life = cooldown;
		parts[i].tmp |= 0x1;
		parts[s].life = rand()%480+480;
		parts[s].tmp = 0x1;
		parts[s].temp = parts[i].temp + 20;
		if (tmp & 2)
		{
			parts[s].ctype = 0x1F<<(rand()%26);
		}
	}
	return 0;
}

Element_E187::~Element_E187() {}
