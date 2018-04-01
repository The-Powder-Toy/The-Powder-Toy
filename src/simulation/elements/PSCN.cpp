#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSCN PT_PSCN 35
Element_PSCN::Element_PSCN()
{
	Identifier = "DEFAULT_PT_PSCN";
	Name = "PSCN";
	Colour = PIXPACK(0x805050);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Meltable = 1;
	Hardness = 1;
	PhotonReflectWavelengths = 0x00000000;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "P-Type Silicon, Will transfer current to any conductor.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1687.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = NULL;
}

Element_PSCN::~Element_PSCN() {}
