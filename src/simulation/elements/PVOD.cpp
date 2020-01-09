#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PVOD()
{
	Identifier = "DEFAULT_PT_PVOD";
	Name = "PVOD";
	Colour = PIXPACK(0x792020);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
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

	HeatConduct = 251;
	Description = "Powered VOID. When activated, destroys entering particles.";

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
	int r, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SPRK)
				{
					if (parts[ID(r)].life>0 && parts[ID(r)].life<4)
					{
						if (parts[ID(r)].ctype==PT_PSCN)
							parts[i].life = 10;
						else if (parts[ID(r)].ctype==PT_NSCN)
							parts[i].life = 9;
					}
				}
				else if (TYP(r)==PT_PVOD)
				{
					if (parts[i].life==10&&parts[ID(r)].life<10&&parts[ID(r)].life>0)
						parts[i].life = 9;
					else if (parts[i].life==0&&parts[ID(r)].life==10)
						parts[i].life = 10;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*16);
	*colr += lifemod;
	return 0;
}
