#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MALM PT_MALM 184
Element_MALM::Element_MALM()
{
Identifier = "DEFAULT_PT_MALM";
	Name = "MALM";
	Colour = PIXPACK(0xD2CDCD);
	MenuVisible = 0;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness =15;

	Weight = 60;

	Temperature = R_TEMP+0.0f	+935.0f;
	HeatConduct = 251;
	Description = "Molten aluminum";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 932.5f;
	LowTemperatureTransition = PT_ALMN;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_MALM::~Element_MALM() {}
