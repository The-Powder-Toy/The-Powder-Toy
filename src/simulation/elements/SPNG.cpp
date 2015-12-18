#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SPNG PT_SPNG 90
Element_SPNG::Element_SPNG()
{
	Identifier = "DEFAULT_PT_SPNG";
	Name = "SPNG";
	Colour = PIXPACK(0xFFBE30);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.00f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.00f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Sponge, absorbs water. Is not a moving solid.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2730.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &Element_SPNG::update;
	Graphics = &Element_SPNG::graphics;
}

//#TPT-Directive ElementHeader Element_SPNG static int update(UPDATE_FUNC_ARGS)
int Element_SPNG::update(UPDATE_FUNC_ARGS)
{
	int r, trade, rx, ry, tmp, np;
	int limit = 50;
	if (parts[i].life<limit && sim->pv[y/CELL][x/CELL]<=3&&sim->pv[y/CELL][x/CELL]>=-3&&parts[i].temp<=374.0f)
	{
		int absorbChanceDenom = parts[i].life*10000/limit + 500;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					switch (r&0xFF)
					{
					case PT_WATR:
					case PT_DSTW:
					case PT_FRZW:
						if (parts[i].life<limit && 500>rand()%absorbChanceDenom)
						{
							parts[i].life++;
							sim->kill_part(r>>8);
						}
						break;
					case PT_SLTW:
						if (parts[i].life<limit && 50>rand()%absorbChanceDenom)
						{
							parts[i].life++;
							if (rand()%4)
								sim->kill_part(r>>8);
							else
								sim->part_change_type(r>>8, x+rx, y+ry, PT_SALT);
						}
						break;
					case PT_CBNW:
						if (parts[i].life<limit && 100>rand()%absorbChanceDenom)
						{
							parts[i].life++;
							sim->part_change_type(r>>8, x+rx, y+ry, PT_CO2);
						}
						break;
					case PT_PSTE:
						if (parts[i].life<limit && 20>rand()%absorbChanceDenom)
						{
							parts[i].life++;
							sim->create_part(r>>8, x+rx, y+ry, PT_CLST);
						}
						break;
					default:
						continue;
					}
				}
	}
	else
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((!r)&&parts[i].life>=1)//if nothing then create water
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_WATR);
						if (np>-1) parts[i].life--;
					}
				}
	for ( trade = 0; trade<9; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_SPNG&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
			{
				tmp = parts[i].life - parts[r>>8].life;
				if (tmp ==1)
				{
					parts[r>>8].life ++;
					parts[i].life --;
					trade = 9;
				}
				else if (tmp>0)
				{
					parts[r>>8].life += tmp/2;
					parts[i].life -= tmp/2;
					trade = 9;
				}
			}
		}
	}
	tmp = 0;
	if (parts[i].life>0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_FIRE)
					{
						tmp++;
						if (parts[r>>8].life>60)
							parts[r>>8].life -= parts[r>>8].life/60;
						else if (parts[r>>8].life>2)
							parts[r>>8].life--;
					}
				}
	}
	if (tmp && parts[i].life>3)
		parts[i].life -= parts[i].life/3;
	if (tmp>1)
		tmp = tmp/2;
	if (tmp || parts[i].temp>=374)
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((!r)&&parts[i].life>=1)//if nothing then create steam
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_WTRV);
						if (np>-1)
						{
							parts[np].temp = parts[i].temp;
							tmp--;
							parts[i].life--;
							parts[i].temp -= 20.0f;
						}
					}
				}
	if (tmp>0)
	{
		if (parts[i].life>tmp)
			parts[i].life -= tmp;
		else
			parts[i].life = 0;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_SPNG static int graphics(GRAPHICS_FUNC_ARGS)
int Element_SPNG::graphics(GRAPHICS_FUNC_ARGS)

{
	*colr -= cpart->life*15;
	*colg -= cpart->life*15;
	*colb -= cpart->life*15;
	if (*colr<=50)
		*colr = 50;
	if (*colg<=50)
		*colg = 50;
	if (*colb<=20)
		*colb = 20;
	return 0;
}


Element_SPNG::~Element_SPNG() {}
