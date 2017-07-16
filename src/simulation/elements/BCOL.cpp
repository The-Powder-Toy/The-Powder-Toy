#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_BCOL PT_BCOL 73
Element_BCOL::Element_BCOL()
{
	Identifier = "DEFAULT_PT_BCOL";
	Name = "BCOL";
	Colour = PIXPACK(0x333333);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;
	PhotonReflectWavelengths = 0x00000000;

	Weight = 90;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 150;
	Description = "Broken Coal. Heavy particles, burns slowly.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_COAL::update;
	Graphics = &Element_COAL::graphics;
}

Element_BCOL::~Element_BCOL() {}
