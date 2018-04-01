#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NSCN PT_NSCN 36
Element_NSCN::Element_NSCN()
{
	Identifier = "DEFAULT_PT_NSCN";
	Name = "NSCN";
	Colour = PIXPACK(0x505080);
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
	Description = "N-Type Silicon, Will not transfer current to P-Type Silicon.";

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

Element_NSCN::~Element_NSCN() {}
