#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_TTAN PT_TTAN 144
Element_TTAN::Element_TTAN()
{
	Identifier = "DEFAULT_PT_TTAN";
	Name = "TTAN";
	Colour = PIXPACK(0x909090);
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
	Description = "Titanium, Higher melting temperature than other metals, blocks all air pressure";

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

	Update = &Element_TTAN::update;
	
}

//#TPT-Directive ElementHeader Element_TTAN static int update(UPDATE_FUNC_ARGS)
int Element_TTAN::update(UPDATE_FUNC_ARGS)
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
					if((pmap[y+ry][x+rx]&0xFF)==PT_TTAN)
						ttan++;
				}
			}
		}

	if(ttan>=2) {
		sim->air->bmap_blockair[y/CELL][x/CELL] = 1;
		sim->air->bmap_blockairh[y/CELL][x/CELL] = 1;
	}
	return 0;
}


Element_TTAN::~Element_TTAN() {}
