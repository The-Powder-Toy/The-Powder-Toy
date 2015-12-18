#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LNTG PT_LNTG 37
Element_LNTG::Element_LNTG()
{
	Identifier = "DEFAULT_PT_LNTG";
	Name = "LN2";
	Colour = PIXPACK(0x80A0DF);
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 30;

	Temperature = 70.15f;
	HeatConduct = 70;
	Description = "Liquid Nitrogen. Very cold, disappears whenever it touches anything warmer.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 63.0f;
	LowTemperatureTransition = PT_NICE;
	HighTemperature = 77.0f;
	HighTemperatureTransition = PT_NONE;

	Update = NULL;
}

Element_LNTG::~Element_LNTG() {}
