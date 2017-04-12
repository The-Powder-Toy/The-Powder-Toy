#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_C5 PT_C5 130
Element_C5::Element_C5()
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

	Temperature = R_TEMP+0.0f	+273.15f;
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

	Update = &Element_C5::update;
	Graphics = &Element_C5::graphics;
}

//#TPT-Directive ElementHeader Element_C5 static int update(UPDATE_FUNC_ARGS)
int Element_C5::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)!=PT_C5 && parts[r>>8].temp<100 && sim->elements[r&0xFF].HeatConduct && ((r&0xFF)!=PT_HSWC||parts[r>>8].life==10)) || (r&0xFF)==PT_CFLM)
				{
					if (!(rand()%6))
					{
						sim->part_change_type(i,x,y,PT_CFLM);
						parts[r>>8].temp = parts[i].temp = 0;
						parts[i].life = rand()%150+50;
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

//#TPT-Directive ElementHeader Element_C5 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_C5::graphics(GRAPHICS_FUNC_ARGS)

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

Element_C5::~Element_C5() {}
