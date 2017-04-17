#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_INDC PT_INDC 193
Element_INDC::Element_INDC()
{
	Identifier = "DEFAULT_PT_INDC";
	Name = "INDC";
	Colour = PIXPACK(0x404060);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Indestructible Conductor.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;
	Properties2 = PROP_NODESTRUCT;

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

Element_INDC::~Element_INDC() {}
