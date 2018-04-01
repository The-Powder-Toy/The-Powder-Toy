#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PLEX PT_PLEX 11
Element_PLEX::Element_PLEX()
{
	Identifier = "DEFAULT_PT_PLEX";
	Name = "C-4";
	Colour = PIXPACK(0xD080E0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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

	Flammable = 1000;
	Explosive = 2;
	Meltable = 50;
	Hardness = 1;
	PhotonReflectWavelengths = 0x1F00003E;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 88;
	Description = "Solid pressure sensitive explosive.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = NULL;
}

Element_PLEX::~Element_PLEX() {}
