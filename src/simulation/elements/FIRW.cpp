#include "simulation/Elements.h"
extern "C"
{
	#include "hmap.h"
}
//#TPT-Directive ElementClass Element_FIRW PT_FIRW 69
Element_FIRW::Element_FIRW()
{
	Identifier = "DEFAULT_PT_FIRW";
	Name = "FIRW";
	Colour = PIXPACK(0xFFA040);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 55;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Fireworks! Colorful, set off by fire.";

	Properties = TYPE_PART|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FIRW::update;
	Graphics = &Element_FIRW::graphics;
}

//#TPT-Directive ElementHeader Element_FIRW static int update(UPDATE_FUNC_ARGS)
int Element_FIRW::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, np;
	if (parts[i].tmp<=0) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
					{
						float gx, gy, multiplier;
						sim->GetGravityField(x, y, sim->elements[PT_FIRW].Gravity, 1.0f, gx, gy);
						if (gx*gx+gy*gy < 0.001f)
						{
							float angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
							gx += sinf(angle)*sim->elements[PT_FIRW].Gravity*0.5f;
							gy += cosf(angle)*sim->elements[PT_FIRW].Gravity*0.5f;
						}
						parts[i].tmp = 1;
						parts[i].life = rand()%10+20;
						multiplier = (parts[i].life+20)*0.2f/sqrtf(gx*gx+gy*gy);
						parts[i].vx -= gx*multiplier;
						parts[i].vy -= gy*multiplier;
						return 0;
					}
				}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life<=0) {
			parts[i].tmp=2;
		} else {
			parts[i].flags &= ~FLAG_STAGNANT;
		}
	}
	else //if (parts[i].tmp>=2)
	{
		float angle, magnitude;
		int caddress = (rand()%200)*3;
		int n;
		unsigned col = (((firw_data[caddress]))<<16) | (((firw_data[caddress+1]))<<8) | ((firw_data[caddress+2]));
		for (n=0; n<40; n++)
		{
			np = sim->create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = ((rand()%60)+40)*0.05f;
				angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx*0.5f + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy*0.5f + sinf(angle)*magnitude;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = rand()%40+70;
				parts[np].temp = (rand()%500)+5750.0f;
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		sim->pv[y/CELL][x/CELL] += 8.0f;
		sim->kill_part(i);
		return 1;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_FIRW static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FIRW::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->tmp > 0)
	{
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}


Element_FIRW::~Element_FIRW() {}
