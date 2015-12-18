#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CNCT PT_CNCT 24
Element_CNCT::Element_CNCT()
{
	Identifier = "DEFAULT_PT_CNCT";
	Name = "CNCT";
	Colour = PIXPACK(0xC0C0C0);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 55;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 100;
	Description = "Concrete, stronger than stone.";

	Properties = TYPE_PART|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1123.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = NULL;
}

Element_CNCT::~Element_CNCT() {}
