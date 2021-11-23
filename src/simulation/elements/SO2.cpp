#include "simulation/ElementCommon.h"

void Element::Element_SO2()
{
	Identifier = "DEFAULT_PT_SO2";
	Name = "SOXY";
	Colour = PIXPACK(0x80A0EF);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 10000;
	Explosive = 25;
	Meltable = 0;
	Hardness = 0;

	Weight = 30;

	DefaultProperties.temp = 80.0f;
	HeatConduct = 70;
	Description = "Solid Oxygen. Very cold. Reacts with fire.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 218.43;
	LowTemperatureTransition = PT_SO2;
	HighTemperature = 90.1f;
	HighTemperatureTransition = PT_O2;
}
