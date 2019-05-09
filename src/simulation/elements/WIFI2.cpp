#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_THMM PT_THMM 199
Element_THMM::Element_THMM()
{
	Identifier = "DEFAULT_PT_THMM";
	Name = "THMM";
	Colour = PIXPACK(0x40A060);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
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
	Hardness = 2;

	Weight = 100;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 232;
	Description = "Thermometer,Changes colour based on its temperature.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
	Graphics = &Element_THMM::graphics;
}

#define FREQUENCY1 0.0928f

//#TPT-Directive ElementHeader Element_THMM static int graphics(GRAPHICS_FUNC_ARGS)
int Element_THMM::graphics(GRAPHICS_FUNC_ARGS)
{
	int q = (int)((cpart->temp)/50);
	*colr = sin(FREQUENCY1*q + 2) * 127 + 128;
	*colg = sin(FREQUENCY1*q + 4) * 127 + 128;
	*colb = sin(FREQUENCY1*q + 8) * 127 + 128;
	return 0;
}


Element_THMM::~Element_THMM() {}
