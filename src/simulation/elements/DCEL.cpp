#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DCEL PT_DCEL 138
Element_DCEL::Element_DCEL()
{
	Identifier = "DEFAULT_PT_DCEL";
	Name = "DCEL";
	Colour = PIXPACK(0x99CC00);
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
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Decelerator, slows down nearby elements.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DCEL::update;
	Graphics = &Element_DCEL::graphics;
}

//#TPT-Directive ElementHeader Element_DCEL static int update(UPDATE_FUNC_ARGS)
int Element_DCEL::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	float multiplier = 1.0f/1.1f;
	if (parts[i].life!=0)
	{
		multiplier = 1.0f - ((parts[i].life > 100 ? 100 : (parts[i].life < 0 ? 0 : parts[i].life)) / 100.0f);
	}
	parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry) && !(rx && ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[ID(r)].vx *= multiplier;
					parts[ID(r)].vy *= multiplier;
					parts[i].tmp = 1;
				}
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_DCEL static int graphics(GRAPHICS_FUNC_ARGS)
int Element_DCEL::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}


Element_DCEL::~Element_DCEL() {}
