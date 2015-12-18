#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VRSS PT_VRSS 175
Element_VRSS::Element_VRSS()
{
	Identifier = "DEFAULT_PT_VRSS";
	Name = "VRSS";
	Colour = PIXPACK(0xD408CD);
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
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
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP+ 273.15f;
	HeatConduct = 251;
	Description = "Solid Virus. Turns everything it touches into virus.";

	Properties = TYPE_SOLID|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 305.0f;
	HighTemperatureTransition = PT_VIRS;

	Update = &Element_VIRS::update;
	Graphics = &Element_VRSS::graphics;
}

//#TPT-Directive ElementHeader Element_VRSS static int graphics(GRAPHICS_FUNC_ARGS)
int Element_VRSS::graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= NO_DECO;
	return 1;
}

Element_VRSS::~Element_VRSS() {}
