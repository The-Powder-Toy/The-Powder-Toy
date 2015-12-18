#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FRZZ PT_FRZZ 100
Element_FRZZ::Element_FRZZ()
{
	Identifier = "DEFAULT_PT_FRZZ";
	Name = "FRZZ";
	Colour = PIXPACK(0xC0E0FF);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.90f;
	Collision = -0.1f;
	Gravity = 0.05f;
	Diffusion = 0.01f;
	HotAir = -0.00005f* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 50;

	Temperature = 253.15f;
	HeatConduct = 46;
	Description = "Freeze powder. When melted, forms ice that always cools. Spreads with regular water.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 1.8f;
	HighPressureTransition = PT_SNOW;
	LowTemperature = 50.0f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 273.15;
	HighTemperatureTransition = PT_FRZW;

	Update = &Element_FRZZ::update;
}

//#TPT-Directive ElementHeader Element_FRZZ static int update(UPDATE_FUNC_ARGS)
int Element_FRZZ::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR && !(rand()%20))
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_FRZW);
					parts[r>>8].life = 100;
					sim->kill_part(i);
					return 1;
				}
			}
	return 0;
}


Element_FRZZ::~Element_FRZZ() {}
