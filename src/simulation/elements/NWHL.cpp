#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NWHL PT_NWHL 151
Element_NWHL::Element_NWHL()
{
	Identifier = "DEFAULT_PT_NWHL";
	Name = "WHOL";
	Colour = PIXPACK(0xFFFFFF);
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "White hole, pushes away other particles with gravity. (Requires Newtonian gravity)";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_NWHL::update;
}

//#TPT-Directive ElementHeader Element_NWHL static int update(UPDATE_FUNC_ARGS)
int Element_NWHL::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp)
		sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] -= restrict_flt(0.001f*parts[i].tmp, 0.1f, 51.2f);
	else
		sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] -= 0.1f;
	return 0;
}


Element_NWHL::~Element_NWHL() {}
