#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RAYT PT_RAYT 186
Element_RAYT::Element_RAYT() {
	Identifier = "DEFAULT_PT_RAYT";
	Name = "RAYT";
	Colour = PIXPACK(0x66ff66);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = 283.15f;
	HeatConduct = 0;
	Description = "RAYT scans in a certain direction until it detects a particle";

	Properties = TYPE_SOLID | PROP_DRAWONCTYPE | PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RAYT::update;
}

#define INVERT_FILTER 0
#define IGNORE_ENERGY 1

//NOTES:
// life and tmp is used to store the direction RAYT will scan in, and is initially set based on the nearest conductive particle's position
// ctype is used to store the target element, if any. (NONE is treated as a wildcard)
// tmp2 is used for settings (binary flags). The flags are as follows:
// 10: Inverts the CTYPE filter so that the element in ctype is the only thing that doesn't trigger RAYT, instead of the opposite.
// 01: Ignore energy particles

//#TPT-Directive ElementHeader Element_RAYT static int update(UPDATE_FUNC_ARGS)
int Element_RAYT::update(UPDATE_FUNC_ARGS) {
	int rx, ry, r = 0;
	if (parts[i].life == -1 && parts[i].tmp == -1) {
		for (rx=-2; rx<3; rx++) {
			for (ry=-2; ry<3; ry++) {
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				int type = TYP(r);
				if (type == PT_SPRK || type == PT_METL || type == PT_NSCN || type == PT_PSCN || type == PT_INSL) {
					parts[i].life = rx;
					parts[i].tmp = ry;
					return 0;
				}
			}
		}
	} else {
		// Stolen from DRAY
		bool foundParticle = false;
		bool isEnergy = false;
		int p = 0;
		for (int xStep = parts[i].life*-1, yStep = parts[i].tmp*-1, xCurrent = x+xStep, yCurrent = y+yStep; ; xCurrent+=xStep, yCurrent+=yStep) {
			int rr;
			// haven't found a particle yet, keep looking for one
			// the first particle it sees decides whether it will copy energy particles or not
			if (!foundParticle) {
				rr = pmap[yCurrent][xCurrent];
				if (!rr) {
					rr = sim->photons[yCurrent][xCurrent];
					if (rr) {
						foundParticle = isEnergy = true;
						p = rr;
						break;
					}
				} else {
					foundParticle = true;
					p = rr;
					break;
				}
			}
		}
		if (foundParticle == true) {
			if (isEnergy) {
				if ((parts[i].tmp2 & 1) && ((TYP(p) == TYP(parts[i].ctype)) xor (parts[i].tmp2 & (1 << 1)))) {
					sim->part_change_type(ID(pmap[y+parts[i].tmp][x+parts[i].life]),x+parts[i].life,y+parts[i].tmp,PT_SPRK);
					return 0;
				} else {
					return 0;
				}
			}
			if ((TYP(p) == TYP(parts[i].ctype)) xor (parts[i].tmp2 & (1 << 1))) {
				sim->part_change_type(ID(pmap[y+parts[i].tmp][x+parts[i].life]),x+parts[i].life,y+parts[i].tmp,PT_SPRK);
			}
		}
	}
	return 0;
}

Element_RAYT::~Element_RAYT() {}
