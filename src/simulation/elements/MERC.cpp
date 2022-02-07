#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_MERC()
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

	DefaultProperties.tmp = 10;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, trade, np;
	// Max number of particles that can be condensed into one
	const int absorbScale = 10000;
	// Obscure division by 0 fix
	if (parts[i].temp + 1 == 0)
		parts[i].temp = 0;
	int maxtmp = int(absorbScale/(parts[i].temp + 1))-1;
	if (RNG::Ref().chance(absorbScale%(int(parts[i].temp)+1), int(parts[i].temp)+1))
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
					if (TYP(r)==PT_MERC&& RNG::Ref().chance(1, 3))
					{
						if ((parts[i].tmp + parts[ID(r)].tmp + 1) <= maxtmp)
						{
							parts[i].tmp += parts[ID(r)].tmp + 1;
							sim->kill_part(ID(r));
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
		rx = RNG::Ref().between(-2, 2);
		ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r)==PT_MERC&&(parts[i].tmp>parts[ID(r)].tmp)&&parts[i].tmp>0)//diffusion
			{
				int temp = parts[i].tmp - parts[ID(r)].tmp;
				if (temp ==1)
				{
					parts[ID(r)].tmp ++;
					parts[i].tmp --;
				}
				else if (temp>0)
				{
					parts[ID(r)].tmp += temp/2;
					parts[i].tmp -= temp/2;
				}
			}
		}
	}
	return 0;
}
