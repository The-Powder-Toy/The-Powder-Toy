#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_X001 PT_X001 255

/*
TODO: 
	- Molten version
	- "muted version" a powder created by mixing X001 with ? that is weaker
*/

Element_X001::Element_X001()
{
	Identifier = "DEFAULT_PT_X001";
	Name = "X001";
	Colour = PIXPACK(0x506030);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	Temperature = R_TEMP+4.0f	+273.15f;
	HeatConduct = 251;
	Description = "Experimental element.";

	Properties = PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_X001::update;
	Graphics = &Element_X001::graphics;
}

//#TPT-Directive ElementHeader Element_X001 static int update(UPDATE_FUNC_ARGS)
int Element_X001::update(UPDATE_FUNC_ARGS)
{
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (3>(rand()%200) && (r&0xFF)==PT_E182)
				{
					if (!(rand()%50))
						s = sim->create_part(-3, x, y, PT_ELEC);
					r = sim->photons[y][x];
					if (r)
						s = sim->create_part(-3, x, y, PT_ELEC);
					parts[r>>8].tmp = 0;
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_X001 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_X001::graphics(GRAPHICS_FUNC_ARGS)
{
	return 0;
}

Element_X001::~Element_X001() {}
