#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_EXOT PT_EXOT 145
Element_EXOT::Element_EXOT()
{
	Identifier = "DEFAULT_PT_EXOT";
	Name = "EXOT";
	Colour = PIXPACK(0x247BFE);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.0003f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 46;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 250;
	Description = "Exotic matter. Explodes with excess exposure to electrons. Has many other odd reactions.";

	Properties = TYPE_LIQUID|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_EXOT::update;
	Graphics = &Element_EXOT::graphics;
}

//#TPT-Directive ElementHeader Element_EXOT static int update(UPDATE_FUNC_ARGS)
int Element_EXOT::update(UPDATE_FUNC_ARGS)
{
	int r, rt, rx, ry, trade, tym;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				if (rt == PT_WARP)
				{
					if (parts[r>>8].tmp2>2000 && !(rand()%100))
					{
						parts[i].tmp2 += 100;
					}
				}
				else if (rt == PT_EXOT)
				{
					if (parts[r>>8].ctype == PT_PROT)
						parts[i].ctype = PT_PROT;
					if (parts[r>>8].life == 1500 && !(rand()%1000))
						parts[i].life = 1500;
				}
				else if (rt == PT_LAVA)
				{
					//turn molten TTAN or molten GOLD to molten VIBR
					if (parts[r>>8].ctype == PT_TTAN || parts[r>>8].ctype == PT_GOLD)
					{
						if (!(rand()%10))
						{
							parts[r>>8].ctype = PT_VIBR;
							sim->kill_part(i);
							return 1;
						}
					}
					//molten VIBR will kill the leftover EXOT though, so the VIBR isn't killed later
					else if (parts[r>>8].ctype == PT_VIBR)
					{
						if (!(rand()%1000))
						{
							sim->kill_part(i);
							return 1;
						}
					}
				}
				if (parts[i].tmp > 245 && parts[i].life > 1337)
					if (rt!=PT_EXOT && rt!=PT_BREC && rt!=PT_DMND && rt!=PT_CLNE && rt!=PT_PRTI && rt!=PT_PRTO && rt!=PT_PCLN && rt!=PT_VOID && rt!=PT_NBHL && rt!=PT_WARP)
					{
						sim->create_part(i, x, y, rt);
						return 1;
					}
			}

	parts[i].tmp--;
	parts[i].tmp2--;
	//reset tmp every 250 frames, gives EXOT it's slow flashing effect
	if (parts[i].tmp < 1 || parts[i].tmp > 250)
		parts[i].tmp = 250;

	if (parts[i].tmp2 < 1)
		parts[i].tmp2 = 1;
	else if (parts[i].tmp2 > 6000)
	{
		parts[i].tmp2 = 10000;
		if (parts[i].life < 1001)
		{
			sim->part_change_type(i, x, y, PT_WARP);
			return 1;
		}
	}
	else if(parts[i].life < 1001)
		sim->pv[y/CELL][x/CELL] += (parts[i].tmp2*CFDS)/160000;

	if (sim->pv[y/CELL][x/CELL]>200 && parts[i].temp>9000 && parts[i].tmp2>200)
	{
		parts[i].tmp2 = 6000;
		sim->part_change_type(i, x, y, PT_WARP);
		return 1;
	}
	if (parts[i].tmp2 > 100)
	{
		for (trade = 0; trade < 9; trade++)
		{
			rx = rand()%5-2;
			ry = rand()%5-2;
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_EXOT && (parts[i].tmp2 > parts[r>>8].tmp2) && parts[r>>8].tmp2 >= 0) //diffusion
				{
					tym = parts[i].tmp2 - parts[r>>8].tmp2;
					if (tym == 1)
					{
						parts[r>>8].tmp2++;
						parts[i].tmp2--;
						break;
					}
					if (tym > 0)
					{
						parts[r>>8].tmp2 += tym/2;
						parts[i].tmp2 -= tym/2;
						break;
					}
				}
			}
		}
	}
	if (parts[i].ctype == PT_PROT)
	{
		if (parts[i].temp < 50.0f)
		{
			sim->create_part(i, x, y, PT_CFLM);
			return 1;
		}
		else
			parts[i].temp -= 1.0f;
	}
	else if (parts[i].temp < 273.15f)
	{
		parts[i].vx = 0;
		parts[i].vy = 0;
		sim->pv[y/CELL][x/CELL] -= 0.01;
		parts[i].tmp--;
	}
	return 0;

}

//#TPT-Directive ElementHeader Element_EXOT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_EXOT::graphics(GRAPHICS_FUNC_ARGS)
{
	int q = cpart->temp;
	int b = cpart->tmp;
	int c = cpart->tmp2;
	if (cpart->life < 1001)
	{
		if ((cpart->tmp2 - 1)>rand()%1000)
		{
			float frequency = 0.04045;
			*colr = (sin(frequency*c + 4) * 127 + 150);
			*colg = (sin(frequency*c + 6) * 127 + 150);
			*colb = (sin(frequency*c + 8) * 127 + 150);

			*firea = 100;
			*firer = 0;
			*fireg = 0;
			*fireb = 0;

			*pixel_mode |= PMODE_FLAT;
			*pixel_mode |= PMODE_FLARE;
		}
		else
		{
			float frequency = 0.00045;
			*colr = (sin(frequency*q + 4) * 127 + (b/1.7));
			*colg = (sin(frequency*q + 6) * 127 + (b/1.7));
			*colb = (sin(frequency*q + 8) * 127 + (b/1.7));
			*cola = cpart->tmp / 6; 

			*firea = *cola;
			*firer = *colr;
			*fireg = *colg;
			*fireb = *colb;

			*pixel_mode |= FIRE_ADD;
			*pixel_mode |= PMODE_BLUR;
		}
	}
	else
	{
		float frequency = 0.01300;
		*colr = (sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*colg = (sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*colb = (sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*cola = cpart->tmp / 6;
		*firea = *cola;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*pixel_mode |= FIRE_ADD;
		*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}

Element_EXOT::~Element_EXOT() {}
