#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_YEST PT_YEST 63
Element_YEST::Element_YEST()
{
	Identifier = "DEFAULT_PT_YEST";
	Name = "YEST";
	Colour = PIXPACK(0xEEE0C0);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 15;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 80;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Yeast, grows when warm (~37C).";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 373.0f;
	HighTemperatureTransition = PT_DYST;

	Update = &Element_YEST::update;
}

//#TPT-Directive ElementHeader Element_YEST static int update(UPDATE_FUNC_ARGS)
int Element_YEST::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_DYST && !(rand()%6) && !sim->legacy_enable)
				{
					sim->part_change_type(i,x,y,PT_DYST);
				}
			}
	if (parts[i].temp>303&&parts[i].temp<317) {
		sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_YEST);
	}
	return 0;
}


Element_YEST::~Element_YEST() {}
