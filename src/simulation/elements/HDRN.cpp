#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_HDRN PT_HDRN 207
Element_HDRN::Element_HDRN()
{
	Identifier = "DEFAULT_PT_HDRN";
	Name = "HDRN";
	Colour = PIXPACK(0x1BCCE0);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = -1;
	
	Temperature = R_TEMP+200.0f+273.15f;
	HeatConduct = 251;
	Description = "Hadron.";
	
	State = ST_GAS;
	Properties = TYPE_ENERGY|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_HDRN::update;
	Graphics = &Element_HDRN::graphics;
}

//#TPT-Directive ElementHeader Element_HDRN static int update(UPDATE_FUNC_ARGS)
int Element_HDRN::update(UPDATE_FUNC_ARGS)
 {
	int r, rt, rx, ry, nb, rrx, rry;
	parts[i].pavg[0] = x;
	parts[i].pavg[1] = y;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				{
				if ((r&0xFF)==PT_AMTR)
				{
					sim->part_change_type(i,x,y,PT_SING);
					sim->parts[i].tmp = 999999;
					sim->parts[i].life = 0;
				}
			}
		}
	return 0;
}

//#TPT-Directive ElementHeader Element_HDRN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_HDRN::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_HDRN::~Element_HDRN() {}
