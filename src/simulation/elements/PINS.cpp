#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_PINS PT_PINS 191
Element_PINS::Element_PINS()
{
	Identifier = "DEFAULT_PT_PINS";
	Name = "UMPI";
	Colour = PIXPACK(0x939699);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Unmeltable pressure insulator. blocks all air pressure.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PINS::update;
}

//#TPT-Directive ElementHeader Element_PINS static int update(UPDATE_FUNC_ARGS)
int Element_PINS::update(UPDATE_FUNC_ARGS)
{
	int rx, ry, ttan = 0;
	if(nt<=2)
		ttan = 2;
	else if(parts[i].tmp)
		ttan = 2;
	else if(nt<=6)
		for (rx=-1; rx<2; rx++) {
			for (ry=-1; ry<2; ry++) {
				if ((!rx != !ry) && BOUNDS_CHECK) {
					if((pmap[y+ry][x+rx]&0xFF)==PT_PINS)
						ttan++;
				}
			}
		}

	if(ttan>=2) {
		sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
		sim->air->bmap_blockairh[y/CELL][x/CELL] = 0x8;
	}
	return 0;
}

Element_PINS::~Element_PINS() {}
