#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_THDR PT_THDR 48
Element_THDR::Element_THDR()
{
	Identifier = "DEFAULT_PT_THDR";
	Name = "THDR";
	Colour = PIXPACK(0xFFFFA0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.0f;
	Loss = 0.30f;
	Collision = -0.99f;
	Gravity = 0.6f;
	Diffusion = 0.62f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = 9000.0f		+273.15f;
	HeatConduct = 1;
	Description = "Lightning! Very hot, inflicts damage upon most materials, and transfers current to metals.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_THDR::update;
	Graphics = &Element_THDR::graphics;
}

//#TPT-Directive ElementHeader Element_THDR static int update(UPDATE_FUNC_ARGS)
int Element_THDR::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	bool kill=false;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				if ((sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life==0 && !(rt==PT_WATR||rt==PT_SLTW) && parts[r>>8].ctype!=PT_SPRK)
				{
					parts[r>>8].ctype = parts[r>>8].type;
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					parts[r>>8].life = 4;
					kill=true;
				}
				else if (rt!=PT_CLNE&&rt!=PT_THDR&&rt!=PT_SPRK&&rt!=PT_DMND&&rt!=PT_FIRE)
				{
					sim->pv[y/CELL][x/CELL] += 100.0f;
					if (sim->legacy_enable&&1>(rand()%200))
					{
						parts[i].life = rand()%50+120;
						sim->part_change_type(i,x,y,PT_FIRE);
					}
					else
						kill=true;
				}
			}
	if (kill) {
		sim->kill_part(i);
		return 1;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_THDR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_THDR::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 160;
	*fireg = 192;
	*fireb = 255;
	*firer = 144;
	*pixel_mode |= FIRE_ADD;
	return 1;
}


Element_THDR::~Element_THDR() {}
