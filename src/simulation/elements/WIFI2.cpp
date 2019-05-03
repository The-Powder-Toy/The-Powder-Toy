#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_TEMP PT_TEMP 199
Element_TEMP::Element_TEMP()
{
	Identifier = "DEFAULT_PT_TEMP";
	Name = "TEMP";
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
	HeatConduct = 252;
	Description = "Temperature meter,Changes colour based on its temp.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_TEMP::update;
	Graphics = &Element_TEMP::graphics;
}

//#TPT-Directive ElementHeader Element_TEMP static int update(UPDATE_FUNC_ARGS)
int Element_TEMP::update(UPDATE_FUNC_ARGS)
{
	return 0;
}

#define FREQUENCY1 0.0928f

//#TPT-Directive ElementHeader Element_TEMP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_TEMP::graphics(GRAPHICS_FUNC_ARGS)
{
	int q = (int)((cpart->temp - 73.15f) / 100 + 1);
	*colr = sin(FREQUENCY1*q + 0) * 127 + 128;
	*colg = sin(FREQUENCY1*q + 2) * 127 + 128;
	*colb = sin(FREQUENCY1*q + 4) * 127 + 128;
	*pixel_mode |= EFFECT_DBGLINES;
	return 0;
}


Element_TEMP::~Element_TEMP() {}
