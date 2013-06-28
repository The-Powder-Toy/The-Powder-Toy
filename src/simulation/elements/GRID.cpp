#include "simulation/Elements.h"

//#TPT-Directive ElementClass Element_GRID PT_GRID 194
Element_GRID::Element_GRID()
{
	Identifier = "DEFAULT_PT_GRID";
	Name = "GRID";
	Colour = PIXPACK(0xC0C0C0);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
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
	Meltable = 1;
	Hardness = 50;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Field grid. Repels energy.";

	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_HOT_GLOW|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1941.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_GRID::update;
	
}

//#TPT-Directive ElementHeader Element_GRID static int update(UPDATE_FUNC_ARGS)
int Element_GRID::update(UPDATE_FUNC_ARGS)
 {
	 int r, rx, ry, ri;
	for(ri = 0; ri <= 10; ri++)
	{
		rx = (rand()%21)-10;
		ry = (rand()%21)-10;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				r = sim->photons[y+ry][x+rx];

			if (r && !(sim->elements[r&0xFF].Properties & TYPE_SOLID) && (sim->elements[r&0xFF].Properties & TYPE_ENERGY) && parts[i].life==1)
			{
				parts[i].tmp = 10;
				if(parts[i].tmp2 == 0)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 1 && (r&0xFF)==PT_ELEC||PT_PHOT||PT_NEUT)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 2 && (r&0xFF)==PT_ELEC)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 3 && (r&0xFF)==PT_NEUT)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 4 && (r&0xFF)==PT_PHOT)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 5 && (r&0xFF)==PT_ELEC||PT_PHOT)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 6 && (r&0xFF)==PT_NEUT||PT_ELEC)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
				if(parts[i].tmp2 == 7 && (r&0xFF)==PT_NEUT||PT_PHOT)
				{
				parts[r>>8].vx += isign(rx)*((parts[i].tmp)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].tmp)/10.0f);
				}
			}
		}
	}
	return 0;
}


Element_GRID::~Element_GRID() {}
