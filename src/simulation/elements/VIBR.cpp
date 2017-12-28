#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VIBR PT_VIBR 165
Element_VIBR::Element_VIBR()
{
	Identifier = "DEFAULT_PT_VIBR";
	Name = "VIBR";
	Colour = PIXPACK(0x005000);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.85f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = 273.15f;
	HeatConduct = 251;
	Description = "Vibranium. Stores energy and releases it in violent explosions.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_VIBR::update;
	Graphics = &Element_VIBR::graphics;
}

//#TPT-Directive ElementHeader Element_VIBR static int update(UPDATE_FUNC_ARGS)
int Element_VIBR::update(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rndstore;
	int trade, transfer;
	if (!parts[i].life) //if not exploding
	{
		//Heat absorption code
		if (parts[i].temp > 274.65f)
		{
			parts[i].tmp++;
			parts[i].temp -= 3;
		}
		else if (parts[i].temp < 271.65f)
		{
			parts[i].tmp--;
			parts[i].temp += 3;
		}
		//Pressure absorption code
		if (sim->pv[y/CELL][x/CELL] > 2.5)
		{
			parts[i].tmp += 7;
			sim->pv[y/CELL][x/CELL]--;
		}
		else if (sim->pv[y/CELL][x/CELL] < -2.5)
		{
			parts[i].tmp -= 2;
			sim->pv[y/CELL][x/CELL]++;
		}
		//initiate explosion counter
		if (parts[i].tmp > 1000)
			parts[i].life = 750;
	}
	else //if it is exploding
	{
		//Release sparks before explode
		rndstore = rand();
		if (parts[i].life < 300)
		{
			rx = rndstore%3-1;
			ry = (rndstore>>2)%3-1;
			rndstore = rndstore >> 4;
			r = pmap[y+ry][x+rx];
			if (TYP(r) && TYP(r) != PT_BREC && (sim->elements[TYP(r)].Properties&PROP_CONDUCTS) && !parts[ID(r)].life)
			{
				parts[ID(r)].life = 4;
				parts[ID(r)].ctype = TYP(r);
				sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
			}
		}
		//Release all heat
		if (parts[i].life < 500)
		{
			rx = rndstore%7-3;
			ry = (rndstore>>3)%7-3;
			if(BOUNDS_CHECK)
			{
				r = pmap[y+ry][x+rx];
				if (TYP(r) && TYP(r)!=PT_VIBR  && TYP(r)!=PT_BVBR && sim->elements[TYP(r)].HeatConduct && (TYP(r)!=PT_HSWC||parts[ID(r)].life==10))
				{
					parts[ID(r)].temp += parts[i].tmp*3;
					parts[i].tmp = 0;
				}
			}
		}
		//Explosion code
		if (parts[i].life == 1)
		{
			if (!parts[i].tmp2)
			{
				rndstore = rand();
				int index = sim->create_part(-3,x+((rndstore>>4)&3)-1,y+((rndstore>>6)&3)-1,PT_ELEC);
				if (index != -1)
					parts[index].temp = 7000;
				index = sim->create_part(-3,x+((rndstore>>8)&3)-1,y+((rndstore>>10)&3)-1,PT_PHOT);
				if (index != -1)
					parts[index].temp = 7000;
				int rx = ((rndstore>>12)&3)-1;
				rndstore = rand();
				index = sim->create_part(-1,x+rx-1,y+rndstore%3-1,PT_BREC);
				if (index != -1)
					parts[index].temp = 7000;
				sim->create_part(i, x, y, PT_EXOT);
				parts[i].tmp2 = (rndstore>>2)%1000;
				parts[i].temp=9000;
				sim->pv[y/CELL][x/CELL] += 50;

				return 1;
			}
			else
			{
				parts[i].tmp2 = 0;
				parts[i].temp = 273.15f;
				parts[i].tmp = 0;
			}
		}
	}
	//Neighbor check loop
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (parts[i].life)
				{
					//Makes EXOT around it get tmp to start exploding too
					if ((TYP(r)==PT_VIBR  || TYP(r)==PT_BVBR))
					{
						if (!parts[ID(r)].life)
							parts[ID(r)].tmp += 45;
						else if (parts[i].tmp2 && parts[i].life > 75 && rand()%2)
						{
							parts[ID(r)].tmp2 = 1;
							parts[i].tmp = 0;
						}
					}
					else if (TYP(r)==PT_CFLM)
					{
						parts[i].tmp2 = 1;
						parts[i].tmp = 0;
					}
				}
				else
				{
					//Melts into EXOT
					if (TYP(r) == PT_EXOT && !(rand()%25))
					{
						sim->part_change_type(i, x, y, PT_EXOT);
						return 1;
					}
				}
				//VIBR+ANAR=BVBR
				if (parts[i].type != PT_BVBR && TYP(r) == PT_ANAR)
				{
					sim->part_change_type(i,x,y,PT_BVBR);
					sim->pv[y/CELL][x/CELL] -= 1;
				}
			}
	for (trade = 0; trade < 9; trade++)
	{
		if (!(trade%2))
			rndstore = rand();
		rx = rndstore%7-3;
		rndstore >>= 3;
		ry = rndstore%7-3;
		rndstore >>= 3;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (TYP(r) != PT_VIBR && TYP(r) != PT_BVBR)
				continue;
			if (parts[i].tmp > parts[ID(r)].tmp)
			{
				transfer = parts[i].tmp - parts[ID(r)].tmp;
				parts[ID(r)].tmp += transfer/2;
				parts[i].tmp -= transfer/2;
				break;
			}
		}
	}
	if (parts[i].tmp < 0)
		parts[i].tmp = 0; // only preventing because negative tmp doesn't save
	return 0;
}

//#TPT-Directive ElementHeader Element_VIBR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_VIBR::graphics(GRAPHICS_FUNC_ARGS)
{
	int gradient = cpart->tmp/10;
	if (gradient >= 100 || cpart->life)
	{
		*colr = (int)(fabs(sin(exp((750.0f-cpart->life)/170)))*200.0f);
		if (cpart->tmp2)
		{
			*colg = *colr;
			*colb = 255;
		}
		else
		{
			*colg = 255;
			*colb = *colr;
		}

		*firea = 90;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_BLEND;
	}
	else if (gradient < 100)
	{
		*colr += (int)restrict_flt(gradient*2.0f,0,255);
		*colg += (int)restrict_flt(gradient*2.0f,0,175);
		*colb += (int)restrict_flt(gradient*2.0f,0,255);
		*firea = (int)restrict_flt(gradient*.6f,0,60);
		*firer = *colr/2;
		*fireg = *colg/2;
		*fireb = *colb/2;
		*pixel_mode |= FIRE_BLEND;
	}
	return 0;
}

Element_VIBR::~Element_VIBR() {}
