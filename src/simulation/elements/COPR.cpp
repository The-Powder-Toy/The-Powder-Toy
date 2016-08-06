#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_COPR PT_COPR 180
Element_COPR::Element_COPR()
{
	Identifier = "DEFAULT_PT_COPR";
	Name = "COPR";
	Colour = PIXPACK(0xCB6351);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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
	Description = "Copper";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_HOT_GLOW|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1356.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_COPR::update;
}

//#TPT-Directive ElementHeader Element_COPR static int update(UPDATE_FUNC_ARGS)
int Element_COPR::update(UPDATE_FUNC_ARGS)
{
	const int rad = 4;
	int rx, ry, rry, rrx, r, count = 0;
	float tempAgg = 0;
	for (rx=-1; rx<2; rx++) {
		for (ry=-1; ry<2; ry++) {
			rry = ry * rad;
			rrx = rx * rad;
			if (REAL_BOUNDS_CHECK(x+rrx, y+rry)) {
				r = pmap[y+rry][x+rrx];
				if(r && (sim->elements[r&0xFF].HeatConduct > 0 || (r&0xFF) == PT_COPR)) {
					count++;
					tempAgg += parts[r>>8].temp;
				}
			}
		}
	}

	if(count > 0) {
		parts[i].temp = tempAgg/count;

		for (rx=-1; rx<2; rx++) {
			for (ry=-1; ry<2; ry++) {
				rry = ry * rad;
				rrx = rx * rad;
				if (REAL_BOUNDS_CHECK(x+rrx, y+rry)) {
					r = pmap[y+rry][x+rrx];
					if(r && (sim->elements[r&0xFF].HeatConduct > 0 || (r&0xFF) == PT_COPR)) {
						parts[r>>8].temp = parts[i].temp;
					}
				}
			}
		}
	}

	return 0;
}


Element_COPR::~Element_COPR() {}
