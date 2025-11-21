#include "simulation/ElementCommon.h"

void Element::Element_LCAR()
{
	Identifier = "DEFAULT_PT_LCAR";
	Name = "LCAR";
	Colour = 0xff9c08_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.05f;
	AirDrag = 0.05f * CFDS;
	AirLoss = 0.995f;
	Loss = 0.999f;
	Collision = 0.0f;
	Gravity = 0.12f;
	Diffusion = 0.00f;
	HotAir = 0.000001f * CFDS;
	Falldown = 2;

	Flammable = 6;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 25;

	DefaultProperties.temp = R_TEMP + 28.0f + 273.15f;
	HeatConduct = 44;
	Description = "Liquid caramel.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 343.0f;
	LowTemperatureTransition = PT_SCAR;
	HighTemperature = 1173.0f;
	HighTemperatureTransition = PT_LAVA;
}
