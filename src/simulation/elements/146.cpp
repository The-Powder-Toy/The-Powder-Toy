#include "simulation/Elements.h"
#include "Lang.h"
//#TPT-Directive ElementClass Element_146 PT_146 146
Element_146::Element_146()
{
	Identifier = "DEFAULT_PT_146";
	Name = "BRAN";
	Colour = PIXPACK(0xCCCC00);
	MenuVisible = 0;
	MenuSection = SC_LIFE;
	Enabled = 0;

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

	Temperature = 9000.0f;
	HeatConduct = 40;
	Description = TEXT_ELEM_146_DESC;

	Properties = TYPE_SOLID|PROP_LIFE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_146::~Element_146() {}
