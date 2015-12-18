#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BOYL PT_BOYL 141
Element_BOYL::Element_BOYL()
{
	Identifier = "DEFAULT_PT_BOYL";
	Name = "BOYL";
	Colour = PIXPACK(0x0A3200);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.18f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	Temperature = R_TEMP+2.0f	+273.15f;
	HeatConduct = 42;
	Description = "Boyle, variable pressure gas. Expands when heated.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_BOYL::update;
}

//#TPT-Directive ElementHeader Element_BOYL static int update(UPDATE_FUNC_ARGS)
int Element_BOYL::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	float limit = parts[i].temp / 100;
	if (sim->pv[y / CELL][x / CELL] < limit)
		sim->pv[y / CELL][x / CELL] += 0.001f*(limit - sim->pv[y / CELL][x / CELL]);
	if (sim->pv[y / CELL + 1][x / CELL] < limit)
		sim->pv[y / CELL + 1][x / CELL] += 0.001f*(limit - sim->pv[y / CELL + 1][x / CELL]);
	if (sim->pv[y / CELL - 1][x / CELL] < limit)
		sim->pv[y / CELL - 1][x / CELL] += 0.001f*(limit - sim->pv[y / CELL - 1][x / CELL]);

	sim->pv[y / CELL][x / CELL + 1]	+= 0.001f*(limit - sim->pv[y / CELL][x / CELL + 1]);
	sim->pv[y / CELL + 1][x / CELL + 1] += 0.001f*(limit - sim->pv[y / CELL + 1][x / CELL + 1]);
	sim->pv[y / CELL][x / CELL - 1]	+= 0.001f*(limit - sim->pv[y / CELL][x / CELL - 1]);
	sim->pv[y / CELL - 1][x / CELL - 1] += 0.001f*(limit - sim->pv[y / CELL - 1][x / CELL - 1]);

	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR)
				{
					if (!(rand()%30))
						sim->part_change_type(r>>8,x+rx,y+ry,PT_FOG);
				}
				else if ((r&0xFF)==PT_O2)
				{
					if (!(rand()%9))
					{
						sim->kill_part(r>>8);
						sim->part_change_type(i,x,y,PT_WATR);
						sim->pv[y/CELL][x/CELL] += 4.0;
					}
				}
			}
	return 0;
}


Element_BOYL::~Element_BOYL() {}
