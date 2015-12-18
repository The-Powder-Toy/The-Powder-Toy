#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_IRON PT_IRON 76
Element_IRON::Element_IRON()
{
	Identifier = "DEFAULT_PT_IRON";
	Name = "IRON";
	Colour = PIXPACK(0x707070);
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
	Description = "Rusts with salt, can be used for electrolysis of WATR.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1687.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_IRON::update;
}

//#TPT-Directive ElementHeader Element_IRON static int update(UPDATE_FUNC_ARGS)
int Element_IRON::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life)
		return 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				switch (r&0xFF)
				{
				case PT_SALT:
					if (!(rand()%47))
						goto succ;
					break;
				case PT_SLTW:
					if (!(rand()%67))
						goto succ;
					break;
				case PT_WATR:
					if (!(rand()%1200))
						goto succ;
					break;
				case PT_O2:
					if (!(rand()%250))
						goto succ;
					break;
				case PT_LO2:
					goto succ;
				default:
					break;
				}
			}
	return 0;
succ:
	sim->part_change_type(i,x,y,PT_BMTL);
	parts[i].tmp=(rand()%10)+20;				
	return 0;
}


Element_IRON::~Element_IRON() {}
