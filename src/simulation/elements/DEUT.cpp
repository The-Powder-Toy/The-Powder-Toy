#include "simulation/ElementCommon.h"
#include "common/tpt-minmax.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_DEUT()
{
	Identifier = "DEFAULT_PT_DEUT";
	Name = "DEUT";
	Colour = PIXPACK(0x00153F);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
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

	Weight = 31;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 251;
	Description = "Deuterium oxide. Volume changes with temp, radioactive with neutrons.";

	Properties = TYPE_LIQUID|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 10;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, trade, np;
	float gravtot = fabs(sim->gravy[(y/CELL)*(XRES/CELL)+(x/CELL)])+fabs(sim->gravx[(y/CELL)*(XRES/CELL)+(x/CELL)]);
	// Prevent division by 0
	float temp = std::max(1.0f, (parts[i].temp + 1));
	int maxlife = ((10000/(temp + 1))-1);
	if (RNG::Ref().chance(10000 % static_cast<int>(temp + 1), static_cast<int>(temp + 1)))
		maxlife++;
	// Compress when Newtonian gravity is applied
	// multiplier=1 when gravtot=0, multiplier -> 5 as gravtot -> inf
	maxlife = maxlife*(5.0f - 8.0f/(gravtot+2.0f));
	if (parts[i].life < maxlife)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r || (parts[i].life >=maxlife))
						continue;
					if (TYP(r)==PT_DEUT&& RNG::Ref().chance(1, 3))
					{
						// If neighbour life+1 fits in the free capacity for this particle, absorb neighbour
						// Condition is written in this way so that large neighbour life values don't cause integer overflow
						if (parts[ID(r)].life <= maxlife - parts[i].life - 1)
						{
							parts[i].life += parts[ID(r)].life + 1;
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
					//Leave if there is nothing to do
					if (parts[i].life <= maxlife)
						goto trade;
					r = pmap[y+ry][x+rx];
					if ((!r)&&parts[i].life>=1)//if nothing then create deut
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_DEUT);
						if (np<0) continue;
						parts[i].life--;
						parts[np].temp = parts[i].temp;
						parts[np].life = 0;
					}
				}
trade:
	for ( trade = 0; trade<4; trade ++)
	{
		rx = RNG::Ref().between(-2, 2);
		ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r)==PT_DEUT&&(parts[i].life>parts[ID(r)].life)&&parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[ID(r)].life;
				if (temp ==1)
				{
					parts[ID(r)].life ++;
					parts[i].life --;
				}
				else if (temp>0)
				{
					parts[ID(r)].life += temp/2;
					parts[i].life -= temp/2;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life>=240)
	{
		*firea = 60;
		*firer = *colr += 255;
		*fireg = *colg += 255;
		*fireb = *colb += 255;
		*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	}
	else if(cpart->life>0)
	{
		*colr += cpart->life*1;
		*colg += cpart->life*2;
		*colb += cpart->life*3;
		*pixel_mode |= PMODE_BLUR;
	}
	else
	{
		*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}
