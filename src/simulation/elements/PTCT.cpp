#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PTCT PT_PTCT 46
Element_PTCT::Element_PTCT()
{
	Identifier = "DEFAULT_PT_PTCT";
	Name = "PTCT";
	Colour = PIXPACK(0x405050);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Semi-conductor. Only conducts electricity when cold. (Less than 100C)";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1687.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_PTCT::update;
}

//#TPT-Directive ElementHeader Element_PTCT static int update(UPDATE_FUNC_ARGS)
int Element_PTCT::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp>295.0f)
		parts[i].temp -= 2.5f;
	return 0;
}


Element_PTCT::~Element_PTCT() {}
