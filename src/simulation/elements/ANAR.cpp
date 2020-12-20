#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ANAR()
{
	Identifier = "DEFAULT_PT_ANAR";
	Name = "ANAR";
	Colour = PIXPACK(0xFFFFEE);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = -0.7f;
	AirDrag = -0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.1f;
	Gravity = -0.1f;
	NewtonianGravity = -1.f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 85;

	HeatConduct = 70;
	Description = "Anti-air. Very light dust, which behaves opposite gravity.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;

	//if (parts[i].temp >= 0.23)
	// parts[i].temp --;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_CFLM && RNG::Ref().chance(1, 4))
				{
					sim->part_change_type(i,x,y,PT_CFLM);
					parts[i].life = RNG::Ref().between(50, 199);
					parts[ID(r)].temp = parts[i].temp = 0;
					sim->pv[y/CELL][x/CELL] -= 0.5;
				}
			}
	return 0;
}
