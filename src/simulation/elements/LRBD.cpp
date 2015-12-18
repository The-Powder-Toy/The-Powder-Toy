#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LRBD PT_LRBD 42
Element_LRBD::Element_LRBD()
{
	Identifier = "DEFAULT_PT_LRBD";
	Name = "LRBD";
	Colour = PIXPACK(0xAAAAAA);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000001f* CFDS;
	Falldown = 2;

	Flammable = 1000;
	Explosive = 1;
	Meltable = 0;
	Hardness = 2;

	Weight = 45;

	Temperature = R_TEMP+45.0f+273.15f;
	HeatConduct = 170;
	Description = "Liquid Rubidium.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 311.0f;
	LowTemperatureTransition = PT_RBDM;
	HighTemperature = 961.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = NULL;
}

Element_LRBD::~Element_LRBD() {}
