#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SOAP PT_SOAP 149
Element_SOAP::Element_SOAP()
{
	Identifier = "DEFAULT_PT_SOAP";
	Name = "SOAP";
	Colour = PIXPACK(0xF5F5DC);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

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
	Hardness = 20;

	Weight = 35;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Soap. Creates bubbles, washes off deco color, and cures virus.";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_SOAP::update;
	Graphics = &Element_SOAP::graphics;
}

//#TPT-Directive ElementHeader Element_SOAP static void detach(Simulation * sim, int i)
void Element_SOAP::detach(Simulation * sim, int i)
{
	if ((sim->parts[i].ctype&2) == 2 && sim->parts[i].tmp >= 0 && sim->parts[i].tmp < NPART && sim->parts[sim->parts[i].tmp].type == PT_SOAP)
	{
		if ((sim->parts[sim->parts[i].tmp].ctype&4) == 4)
			sim->parts[sim->parts[i].tmp].ctype ^= 4;
	}

	if ((sim->parts[i].ctype&4) == 4 && sim->parts[i].tmp2 >= 0 && sim->parts[i].tmp2 < NPART && sim->parts[sim->parts[i].tmp2].type == PT_SOAP)
	{
		if ((sim->parts[sim->parts[i].tmp2].ctype&2) == 2)
			sim->parts[sim->parts[i].tmp2].ctype ^= 2;
	}

	sim->parts[i].ctype = 0;
}

//#TPT-Directive ElementHeader Element_SOAP static void attach(Particle * parts, int i1, int i2)
void Element_SOAP::attach(Particle * parts, int i1, int i2)
{
	if (!(parts[i2].ctype&4))
	{
		parts[i1].ctype |= 2;
		parts[i1].tmp = i2;

		parts[i2].ctype |= 4;
		parts[i2].tmp2 = i1;
	}
	else if (!(parts[i2].ctype&2))
	{
		parts[i1].ctype |= 4;
		parts[i1].tmp2= i2;

		parts[i2].ctype |= 2;
		parts[i2].tmp = i1;
	}
}

#define FREEZING 248.15f
#define BLEND 0.85f

//#TPT-Directive ElementHeader Element_SOAP static int update(UPDATE_FUNC_ARGS)
int Element_SOAP::update(UPDATE_FUNC_ARGS)
 
{
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta;

	//0x01 - bubble on/off
	//0x02 - first mate yes/no
	//0x04 - "back" mate yes/no

	if (parts[i].ctype&1)
	{
		// reset invalid SOAP links
		if (parts[i].tmp < 0 || parts[i].tmp >= NPART || parts[i].tmp2 < 0 || parts[i].tmp2 >= NPART)
		{
			parts[i].tmp = parts[i].tmp2 = parts[i].ctype = 0;
			return 0;
		}
		if (parts[i].temp>FREEZING)
		{
			if (parts[i].life<=0)
			{
				//if only connected on one side
				if ((parts[i].ctype&6) != 6 && (parts[i].ctype&6))
				{
					int target = i;
					//break entire bubble in a loop
					while((parts[target].ctype&6) != 6 && (parts[target].ctype&6) && parts[target].type == PT_SOAP)
					{
						if (parts[target].ctype&2)
						{
							target = parts[target].tmp;
							detach(sim, target);
						}
						if (parts[target].ctype&4)
						{
							target = parts[target].tmp2;
							detach(sim, target);
						}
					}
				}
				if ((parts[i].ctype&6) != 6)
					parts[i].ctype = 0;
				if ((parts[i].ctype&6) == 6 && (parts[parts[i].tmp].ctype&6) == 6 && parts[parts[i].tmp].tmp == i)
					detach(sim, i);
			}
			parts[i].vy = (parts[i].vy-0.1f)*0.5f;
			parts[i].vx *= 0.5f;
		}
		if(!(parts[i].ctype&2))
		{
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if ((parts[ID(r)].type == PT_SOAP) && (parts[ID(r)].ctype&1) && !(parts[ID(r)].ctype&4))
							Element_SOAP::attach(parts, i, ID(r));
					}
		}
		else
		{
			if (parts[i].life<=0)
				for (rx=-2; rx<3; rx++)
					for (ry=-2; ry<3; ry++)
						if (BOUNDS_CHECK && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if (!r && !sim->bmap[(y+ry)/CELL][(x+rx)/CELL])
								continue;
							if (parts[i].temp>FREEZING)
							{
								if (sim->bmap[(y+ry)/CELL][(x+rx)/CELL]
									|| (r && !(sim->elements[TYP(r)].Properties&TYPE_GAS)
								    && TYP(r) != PT_SOAP && TYP(r) != PT_GLAS))
								{
									detach(sim, i);
									continue;
								}
							}
							if (TYP(r) == PT_SOAP)
							{
								if (parts[ID(r)].ctype == 1)
								{
									int buf = parts[i].tmp;

									parts[i].tmp = ID(r);
									if (parts[buf].type == PT_SOAP)
										parts[buf].tmp2 = ID(r);
									parts[ID(r)].tmp2 = i;
									parts[ID(r)].tmp = buf;
									parts[ID(r)].ctype = 7;
								}
								else if (parts[ID(r)].ctype == 7 && parts[i].tmp != ID(r) && parts[i].tmp2 != ID(r))
								{
									if (parts[parts[i].tmp].type == PT_SOAP)
										parts[parts[i].tmp].tmp2 = parts[ID(r)].tmp2;
									if (parts[parts[ID(r)].tmp2].type == PT_SOAP)
										parts[parts[ID(r)].tmp2].tmp = parts[i].tmp;
									parts[ID(r)].tmp2 = i;
									parts[i].tmp = ID(r);
								}
							}
						}
		}
		if(parts[i].ctype&2)
		{
			float d, dx, dy;
			dx = parts[i].x - parts[parts[i].tmp].x;
			dy = parts[i].y - parts[parts[i].tmp].y;
			d = 9/(pow(dx, 2)+pow(dy, 2)+9)-0.5;
			parts[parts[i].tmp].vx -= dx*d;
			parts[parts[i].tmp].vy -= dy*d;
			parts[i].vx += dx*d;
			parts[i].vy += dy*d;
			if ((parts[parts[i].tmp].ctype&2) && (parts[parts[i].tmp].ctype&1)
					&& (parts[parts[i].tmp].tmp >= 0 && parts[parts[i].tmp].tmp < NPART)
					&& (parts[parts[parts[i].tmp].tmp].ctype&2) && (parts[parts[parts[i].tmp].tmp].ctype&1))
			{
				int ii = parts[parts[parts[i].tmp].tmp].tmp;
				if (ii >= 0 && ii < NPART)
				{
					dx = parts[ii].x - parts[parts[i].tmp].x;
					dy = parts[ii].y - parts[parts[i].tmp].y;
					d = 81/(pow(dx, 2)+pow(dy, 2)+81)-0.5;
					parts[parts[i].tmp].vx -= dx*d*0.5f;
					parts[parts[i].tmp].vy -= dy*d*0.5f;
					parts[ii].vx += dx*d*0.5f;
					parts[ii].vy += dy*d*0.5f;
				}
			}
		}
	}
	else
	{
		if (sim->pv[y/CELL][x/CELL]>0.5f || sim->pv[y/CELL][x/CELL]<(-0.5f))
		{
			parts[i].ctype = 1;
			parts[i].life = 10;
		}
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r) == PT_OIL)
					{
						float ax, ay;
						ax = (parts[i].vx*0.5f + parts[ID(r)].vx)/2;
						ay = ((parts[i].vy-0.1f)*0.5f + parts[ID(r)].vy)/2;
						parts[i].vx = parts[ID(r)].vx = ax;
						parts[i].vy = parts[ID(r)].vy = ay;
					}
				}
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_SOAP)
				{
					tr = (parts[ID(r)].dcolour>>16)&0xFF;
					tg = (parts[ID(r)].dcolour>>8)&0xFF;
					tb = (parts[ID(r)].dcolour)&0xFF;
					ta = (parts[ID(r)].dcolour>>24)&0xFF;
					nr = (tr*BLEND);
					ng = (tg*BLEND);
					nb = (tb*BLEND);
					na = (ta*BLEND);
					parts[ID(r)].dcolour = nr<<16 | ng<<8 | nb | na<<24;
				}
			}

	return 0;
}

//#TPT-Directive ElementHeader Element_SOAP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_SOAP::graphics(GRAPHICS_FUNC_ARGS)

{
	*pixel_mode |= EFFECT_LINES|PMODE_BLUR;
	return 1;
}

Element_SOAP::~Element_SOAP() {}
