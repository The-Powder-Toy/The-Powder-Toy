#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_SWCH()
{
	Identifier = "DEFAULT_PT_SWCH";
	Name = "SWCH";
	Colour = PIXPACK(0x103B11);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "Only conducts when switched on. (PSCN switches on, NSCN switches off)";

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

static bool isRedBRAY(UPDATE_FUNC_ARGS, int xc, int yc)
{
	return TYP(pmap[yc][xc]) == PT_BRAY && parts[ID(pmap[yc][xc])].tmp == 2;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rt, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->parts_avg(i,ID(r),PT_INSL)!=PT_INSL) {
					rt = TYP(r);
					if (rt==PT_SWCH)
					{
						if (parts[i].life>=10&&parts[ID(r)].life<10&&parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[ID(r)].life>=10)
						{
							//Set to other particle's life instead of 10, otherwise spark loops form when SWCH is sparked while turning on
							parts[i].life = parts[ID(r)].life;
						}
					}
					else if (rt==PT_SPRK && parts[i].life==10 && parts[ID(r)].life>0 && parts[ID(r)].ctype!=PT_PSCN && parts[ID(r)].ctype!=PT_NSCN) {
						sim->part_change_type(i,x,y,PT_SPRK);
						parts[i].ctype = PT_SWCH;
						parts[i].life = 4;
					}
				}
			}
	//turn SWCH on/off from two red BRAYS. There must be one either above or below, and one either left or right to work, and it can't come from the side, it must be a diagonal beam
	if (!TYP(pmap[y-1][x-1]) && !TYP(pmap[y-1][x+1]) && (isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x, y-1) || isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x, y+1)) && (isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x+1, y) || isRedBRAY(UPDATE_FUNC_SUBCALL_ARGS, x-1, y)))
	{
		if (parts[i].life == 10)
			parts[i].life = 9;
		else if (parts[i].life <= 5)
			parts[i].life = 14;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->life >= 10)
	{
		*colr = 17;
		*colg = 217;
		*colb = 24;
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}
