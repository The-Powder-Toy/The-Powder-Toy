#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LED PT_LED 197
Element_LED::Element_LED()
{
	Identifier = "DEFAULT_PT_LED";
	Name = "LED";
	Colour = PIXPACK(0x9EA3B6);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	Temperature = 363.15f;
	HeatConduct = 251;
	Description = "Light emitting diode.PSCN activates,NSCN deactivates.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 375.15f;
	HighTemperatureTransition = PT_LED1;

	Update = &Element_LED::update;
	Graphics = &Element_LED::graphics;
}
//#TPT-Directive ElementHeader Element_LED static int update(UPDATE_FUNC_ARGS)
int Element_LED::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 343.0f)
		parts[i].temp -= 343.15f;
	if (parts[i].temp < 343.0f)
		parts[i].temp += 343.15f;
	return 0;
}

				//#TPT-Directive ElementHeader Element_LED static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LED::graphics(GRAPHICS_FUNC_ARGS) 
{
	return 0;
}

Element_LED::~Element_LED() {}
