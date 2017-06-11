#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FRME PT_FRME 169
Element_FRME::Element_FRME()
{
	Identifier = "DEFAULT_PT_FRME";
	Name = "FRME";
	Colour = PIXPACK(0x999988);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
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
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 0;
	Description = "Frame, can be used with pistons to push many particles.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Graphics = &Element_FRME::graphics;
}

//#TPT-Directive ElementHeader Element_FRME static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FRME::graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
	{
		*colr += 30;
		*colg += 30;
		*colb += 30;
	}
	return 0;
}

Element_FRME::~Element_FRME() {}
