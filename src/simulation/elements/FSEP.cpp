#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FSEP PT_FSEP 71
Element_FSEP::Element_FSEP()
{
	Identifier = "DEFAULT_PT_FSEP";
	Name = "FSEP";
	Colour = PIXPACK(0x63AD5F);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 70;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Fuse Powder. Burns slowly like FUSE.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FSEP::update;
}

//#TPT-Directive ElementHeader Element_FSEP static int update(UPDATE_FUNC_ARGS)
int Element_FSEP::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life<=0) {
		r = sim->create_part(i, x, y, PT_PLSM);
		if (r!=-1)
			parts[r].life = 50;
		return 1;
	} 
	else if (parts[i].life < 40) {
		parts[i].life--;
		if (!(rand()%10)) {
			r = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_PLSM);
			if (r>-1)
				parts[r].life = 50;
		}
	}
	else {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_SPRK || (parts[i].temp>=(273.15+400.0f))) && parts[i].life>40 && !(rand()%15))
					{
						parts[i].life = 39;						
					}
				}
	}
	return 0;
}


Element_FSEP::~Element_FSEP() {}
