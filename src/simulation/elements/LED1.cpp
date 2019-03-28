#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LED1 PT_LED1 198
Element_LED1::Element_LED1()
{
	Identifier = "DEFAULT_PT_LED1";
	Name = "LED1";
	Colour = PIXPACK(0x9EA3B6);
	MenuVisible = 0;
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

	Temperature = 234.15f;
	HeatConduct = 251;
	Description = "Light emitting diode.Glows";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 372.15f;
	LowTemperatureTransition = PT_LED;
	HighTemperature = NT;
	HighTemperatureTransition = NT;

	Update = &Element_LED1::update;
	Graphics = &Element_LED1::graphics;
}
//#TPT-Directive ElementHeader Element_LED1 static int update(UPDATE_FUNC_ARGS)
int Element_LED1::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 235.15f)
		parts[i].temp -= 235.15f;
	if (parts[i].temp < 235.15f)
		parts[i].temp += 235.15f;
	return 0;
}

//#TPT-Directive ElementHeader Element_LED1 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LED1::graphics(GRAPHICS_FUNC_ARGS)

{
	int gradv;
	{
		*firer = (int)(gradv * 158.0);
		*fireg = (int)(gradv * 156.0);
		*fireb = (int)(gradv * 112.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}

Element_LED1::~Element_LED1() {}
