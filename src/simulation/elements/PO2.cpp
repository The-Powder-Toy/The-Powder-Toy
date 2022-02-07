#include "simulation/ElementCommon.h"

void Element::Element_PO2()
{
	Identifier = "DEFAULT_PT_PO2";
	Name = "POXY";
	Colour = PIXPACK(0x80C0AF);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.00f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 10000;
	Explosive = 25;
	Meltable = 0;
	Hardness = 0;

	Weight = 87;

	DefaultProperties.temp = 54.73f;
	HeatConduct = 70;
	Description = "Powdered Oxygen. Very cold. Reacts with fire, turns into LO2/LOXY when under -218.43C.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITH;
	LowTemperatureTransition = NT;
	HighTemperature = 54.72f;
	HighTemperatureTransition = PT_LO2;
}
