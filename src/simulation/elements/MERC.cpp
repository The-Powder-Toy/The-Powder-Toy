#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MERC PT_MERC 152
Element_MERC::Element_MERC()
{
	Identifier = "DEFAULT_PT_MERC";
	Name = "MERC";
	Colour = PIXPACK(0x736B6D);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 91;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Mercury. Volume changes with temperature, Conductive.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTABSORB|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_MERC::update;
}

//#TPT-Directive ElementHeader Element_MERC static int update(UPDATE_FUNC_ARGS)
int Element_MERC::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, trade, np;
	const int absorbScale = 10000;// max number of particles that can be condensed into one
	int maxtmp = ((absorbScale/(parts[i].temp + 1))-1);
	if ((absorbScale%((int)parts[i].temp+1))>rand()%((int)parts[i].temp+1))
		maxtmp ++;

	if (parts[i].tmp < 0)
	{
		parts[i].tmp = 0;
	}
	if (parts[i].tmp > absorbScale)
	{
		parts[i].tmp = absorbScale;
	}

	if (parts[i].tmp < maxtmp)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r || (parts[i].tmp >=maxtmp))
						continue;
					if ((r&0xFF)==PT_MERC&& !(rand()%3))
					{
						if ((parts[i].tmp + parts[r>>8].tmp + 1) <= maxtmp)
						{
							parts[i].tmp += parts[r>>8].tmp + 1;
							sim->kill_part(r>>8);
						}
					}
				}
	}
	else
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (parts[i].tmp<=maxtmp)
						continue;
					if ((!r)&&parts[i].tmp>=1)//if nothing then create MERC
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_MERC);
						if (np<0) continue;
						parts[i].tmp--;
						parts[np].temp = parts[i].temp;
						parts[np].tmp = 0;
						parts[np].dcolour = parts[i].dcolour;
					}
				}
	for ( trade = 0; trade<4; trade ++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_MERC&&(parts[i].tmp>parts[r>>8].tmp)&&parts[i].tmp>0)//diffusion
			{
				int temp = parts[i].tmp - parts[r>>8].tmp;
				if (temp ==1)
				{
					parts[r>>8].tmp ++;
					parts[i].tmp --;
				}
				else if (temp>0)
				{
					parts[r>>8].tmp += temp/2;
					parts[i].tmp -= temp/2;
				}
			}
		}
	}
	return 0;
}


Element_MERC::~Element_MERC() {}
