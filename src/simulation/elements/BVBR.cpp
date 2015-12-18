#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BVBR PT_BVBR 166
Element_BVBR::Element_BVBR()
{
	Identifier = "DEFAULT_PT_BVBR";
	Name = "BVBR";
	Colour = PIXPACK(0x005000);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.0000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 67;

	Temperature = 273.15f;
	HeatConduct = 164;
	Description = "Broken vibranium.";

	Properties = TYPE_PART|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_VIBR::update;
	Graphics = &Element_VIBR::graphics;
}

Element_BVBR::~Element_BVBR() {}
