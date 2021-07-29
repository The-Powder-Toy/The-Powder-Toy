#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_RGOO()
{
	Identifier = "DEFAULT_PT_RGOO";
	Name = "RGOO";
	Colour = PIXPACK(0xaf0000);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
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

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 50;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 60;
	Description = "Red goo. Nanorobots that turn everything into red goo! Spicy edition.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTPASS;

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
	parts[i].tmp = 0;

	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				
				if (!r)
					continue;

				if (TYP(r)==PT_RGOO){
					parts[i].tmp++;
					continue;
				}

				if (!RNG::Ref().chance(parts[i].temp, MAX_TEMP))
					continue;

				sim->part_change_type(ID(r), x, y, PT_RGOO);
			}

	if (parts[i].tmp >= 8){
		sim->part_change_type(i, x, y, PT_PLSM);
	}

	return 0;
}
