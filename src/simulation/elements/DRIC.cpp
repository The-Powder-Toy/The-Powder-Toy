#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DRIC PT_DRIC 81
Element_DRIC::Element_DRIC()
{
	Identifier = "DEFAULT_PT_DRIC";
	Name = "DRIC";
	Colour = PIXPACK(0xE0E0E0);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.0005f* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 100;

	Temperature = 172.65f;
	HeatConduct = 2;
	Description = "Dry Ice, formed when CO2 is cooled.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 195.65f;
	HighTemperatureTransition = PT_CO2;

	Update = NULL;
}

Element_DRIC::~Element_DRIC() {}
