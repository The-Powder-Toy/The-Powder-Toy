#include "simulation/ElementCommon.h"

int Element_RFRG_update(UPDATE_FUNC_ARGS);

void Element::Element_RFGL()
{
	Identifier = "DEFAULT_PT_RFGL";
	Name = "RFGL";
	Colour = PIXPACK(0x84C2CF);
	MenuVisible = 0;
	MenuSection = SC_LIQUID;
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 10;

	HeatConduct = 3;
	Description = "Liquid refrigerant.";

	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = 2;
	LowPressureTransition = PT_RFRG;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RFRG_update;
}
