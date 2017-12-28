#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FUSE PT_FUSE 70
Element_FUSE::Element_FUSE()
{
	Identifier = "DEFAULT_PT_FUSE";
	Name = "FUSE";
	Colour = PIXPACK(0x0A5706);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 200;
	Description = "Burns slowly. Ignites at somewhat high temperatures or with electricity.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FUSE::update;
}

//#TPT-Directive ElementHeader Element_FUSE static int update(UPDATE_FUNC_ARGS)
int Element_FUSE::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life<=0) {
		r = sim->create_part(i, x, y, PT_PLSM);
		if (r>-1)
			parts[r].life = 50;
		return 1;
	}
	else if (parts[i].life < 40) {
		parts[i].life--;
		if (!(rand()%100)) {
			r = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_PLSM);
			if (r>-1)
				parts[r].life = 50;
		}
	}
	if ((sim->pv[y/CELL][x/CELL] > 2.7f) && parts[i].tmp>40)
		parts[i].tmp=39;
	else if (parts[i].tmp<=0) {
		sim->create_part(i, x, y, PT_FSEP);
		return 1;
	}
	else if (parts[i].tmp<40)
		parts[i].tmp--;

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SPRK || (parts[i].temp>=(273.15+700.0f) && !(rand()%20)))
				{
					if (parts[i].life > 40)
						parts[i].life = 39;
				}
			}
	return 0;
}


Element_FUSE::~Element_FUSE() {}
