#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E185 PT_E185 185
Element_E185::Element_E185()
{
	Identifier = "DEFAULT_PT_E185";
	Name = "E185";
	Colour = PIXPACK(0x20FF90);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -1.00f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = -1;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 0;
	Description = "Experimental photon.";

	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E185::update;
}

//#TPT-Directive ElementHeader Element_E185 static int update(UPDATE_FUNC_ARGS)
int Element_E185::update(UPDATE_FUNC_ARGS)
{
	return 0;
}

Element_E185::~Element_E185() {}
