#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_C5()
{
	Identifier = "DEFAULT_PT_C5";
	Name = "C-5";
	Colour = PIXPACK(0x2050E0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 88;
	Description = "Cold explosive, set off by anything cold.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE | PROP_LIFE_DEC;

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
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((TYP(r)!=PT_C5 && parts[ID(r)].temp<100 && sim->elements[TYP(r)].HeatConduct && (TYP(r)!=PT_HSWC||parts[ID(r)].life==10)) || TYP(r)==PT_CFLM)
				{
					if (RNG::Ref().chance(1, 6))
					{
						sim->part_change_type(i,x,y,PT_CFLM);
						parts[ID(r)].temp = parts[i].temp = 0;
						parts[i].life = RNG::Ref().between(50, 199);
						sim->pv[y/CELL][x/CELL] += 1.5;
					}
				}
			}
	if (parts[i].ctype && !parts[i].life)
	{
		float vx = ((parts[i].tmp << 16) >> 16) / 255.0f;
		float vy = (parts[i].tmp >> 16) / 255.0f;
		float dx = ((parts[i].tmp2 << 16) >> 16) / 255.0f;
		float dy = (parts[i].tmp2 >> 16) / 255.0f;
		r = sim->create_part(-3, x, y, PT_PHOT);
		if (r != -1)
		{
			parts[r].ctype = parts[i].ctype;
			parts[r].x += dx;
			parts[r].y += dy;
			parts[r].vx = vx;
			parts[r].vy = vy;
			parts[r].temp = parts[i].temp;
		}
		parts[i].ctype = 0;
		parts[i].tmp = 0;
		parts[i].tmp2 = 0;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(!cpart->ctype)
		return 0;

	int x = 0;
	*colr = *colg = *colb = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;

	*firea = 100;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD | NO_DECO;
	return 0;
}
