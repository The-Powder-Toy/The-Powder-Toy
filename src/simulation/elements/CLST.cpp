#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CLST PT_CLST 155
Element_CLST::Element_CLST()
{
	Identifier = "DEFAULT_PT_CLST";
	Name = "CLST";
	Colour = PIXPACK(0xE4A4A4);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.2f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 55;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Clay dust. Produces paste when mixed with water.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1256.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_CLST::update;
	Graphics = &Element_CLST::graphics;
}

//#TPT-Directive ElementHeader Element_CLST static int update(UPDATE_FUNC_ARGS)
int Element_CLST::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	float cxy = 0;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR)
				{
					if (!(rand()%1500))
					{
						sim->create_part(i, x, y, PT_PSTS);
						sim->kill_part(r>>8);
					}
				}
				else if ((r&0xFF)==PT_NITR)
				{
					sim->create_part(i, x, y, PT_BANG);
					sim->create_part(r>>8, x+rx, y+ry, PT_BANG);
				}
				else if ((r&0xFF)==PT_CLST)
				{
					if(parts[i].temp <195)
						cxy = 0.05;
					else if(parts[i].temp <295)
						cxy = 0.015;
					else if(parts[i].temp <350)
						cxy = 0.01;
					else
						cxy = 0.005;
					parts[i].vx += cxy*rx;
					parts[i].vy += cxy*ry;//These two can be set not to calculate over 350 later. They do virtually nothing over 0.005.
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_CLST static int graphics(GRAPHICS_FUNC_ARGS)
int Element_CLST::graphics(GRAPHICS_FUNC_ARGS)

{
	int z = (cpart->tmp - 5) * 16;//speckles!
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}


Element_CLST::~Element_CLST() {}
