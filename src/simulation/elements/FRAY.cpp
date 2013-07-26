#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FRAY PT_FRAY 159
Element_FRAY::Element_FRAY()
{
	Identifier = "DEFAULT_PT_FRAY";
	Name = "FRAY";
	Colour = PIXPACK(0x00BBFF);
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
	HotAir = 0.000f  * CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = 20.0f+0.0f +273.15f;
	HeatConduct = 0;
	Description = "Force Emitter. Pushes or pulls objects based on its temperature. Use like ARAY.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_FRAY::update;
	
}

//#TPT-Directive ElementHeader Element_FRAY static int update(UPDATE_FUNC_ARGS)
int Element_FRAY::update(UPDATE_FUNC_ARGS)
 {
	int r, nxx, nyy, len, nxi, nyi, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK) {
					for (nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1, len = 0; ; nyy+=nyi, nxx+=nxi, len++) {
						if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0) || len>10) {
							break;
						}
						r = pmap[y+nyi+nyy][x+nxi+nxx];
						if (!r)
							r = sim->photons[y+nyi+nyy][x+nxi+nxx];
						if (r && !(sim->elements[r&0xFF].Properties & TYPE_SOLID)){
							parts[r>>8].vx += nxi*((parts[i].temp-273.15)/10.0f);
							parts[r>>8].vy += nyi*((parts[i].temp-273.15)/10.0f);
						}
					}
				}
			}
	return 0;
}


Element_FRAY::~Element_FRAY() {}
