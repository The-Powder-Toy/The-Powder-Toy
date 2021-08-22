#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_ABTR()
{
	Identifier = "DEFAULT_PT_ABTR";
	Name = "ABTR";
	Colour = PIXPACK(0x3D3D3D);
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;
	DefaultProperties.life = 100;

	Weight = 100;

	HeatConduct = 1;
	Description = "Ablator. Useful for short bursts of heat resistance";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, trade, rx, ry, tmp, np;
	int limit = 50;


	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (RNG::Ref().chance(parts[i].temp, 10000))
				{
					if ((!r)&&parts[i].life>=1)//if nothing then create co2
					{
						np = sim->create_part(-1,x+rx,y+ry,PT_CO2);
						if (np>-1) parts[i].life--;
					}
				}
			}
			r = pmap[y+ry][x+rx];
			if (TYP(r)==PT_FIRE || TYP(r)==PT_PLSM)
			{
				parts[i].life--;
				if (RNG::Ref().chance(parts[i].temp, 10000))
				{
					sim->part_change_type(i, x, y, PT_CO2);
					return 1;
				}
			}
		}
	for ( trade = 0; trade<9; trade ++)
	{
		rx = RNG::Ref().between(-2, 2);
		ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r)==PT_SPNG&&(parts[i].life>parts[ID(r)].life)&&parts[i].life>0)//diffusion
			{
				tmp = parts[i].life - parts[ID(r)].life;
				if (tmp ==1)
				{
					parts[ID(r)].life ++;
					parts[i].life --;
				}
				else if (tmp>0)
				{
					parts[ID(r)].life += tmp/2;
					parts[i].life -= tmp/2;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*colr += cpart->life*0.61;
	*colg += cpart->life*0.61;
	*colb += cpart->life*0.61;
	if (*colr<=61)
		*colr = 61;
	if (*colg<=61)
		*colg = 61;
	if (*colb<=61)
		*colb = 61;
	return 0;
}
