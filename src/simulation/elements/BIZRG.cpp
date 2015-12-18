#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BIZRG PT_BIZRG 104
Element_BIZRG::Element_BIZRG()
{
	Identifier = "DEFAULT_PT_BIZRG";
	Name = "BIZG";
	Colour = PIXPACK(0x00FFBB);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 2.75f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	Temperature = R_TEMP-200.0f+273.15f;
	HeatConduct = 42;
	Description = "Bizarre gas.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 100.0f;
	HighTemperatureTransition = PT_BIZR;

	Update = &Element_BIZR::update;
	Graphics = &Element_BIZR::graphics;
}
//BIZRG update is in BIZR.cpp
Element_BIZRG::~Element_BIZRG() {}
