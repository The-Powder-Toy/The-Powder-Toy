#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_OIL PT_OIL 3
Element_OIL::Element_OIL()
{
	Identifier = "DEFAULT_PT_OIL";
	Name = "OIL";
	Colour = PIXPACK(0x404010);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 20;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 42;
	Description = "Flammable, turns into GAS at low pressure or high temperature. Can be formed with NEUT and NITR.";

	Properties = TYPE_LIQUID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 333.0f;
	HighTemperatureTransition = PT_GAS;

	Update = NULL;
}

Element_OIL::~Element_OIL() {}
