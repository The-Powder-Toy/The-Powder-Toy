#include "simulation/ElementCommon.h"

void Element::Element_TVKY()
{
	Identifier = "DEFAULT_PT_TVKY";
	Name = "SDO2";
	Colour = PIXPACK(0x80A0AF);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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

	Flammable = 10000;
	Explosive = 25;
	Meltable = 0;
	Hardness = 0;

	Weight = 30;

	DefaultProperties.temp = 54.73f;
	HeatConduct = 70;
	Description = "Solid Oxygen. Very cold. Reacts with fire, turns back into LO2/LOXY when under -218.43C.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 57.5f;
	HighPressureTransition = PT_PO2;
	LowTemperature = ITH;
	LowTemperatureTransition = NT;
	HighTemperature = 54.72f;
	HighTemperatureTransition = PT_LO2;
}
