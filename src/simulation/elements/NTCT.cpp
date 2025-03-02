#include "simulation/ElementCommon.h"
#include "NTCT.h"

void Element::Element_NTCT()
{
	Identifier = "DEFAULT_PT_NTCT";
	Name = "NTCT";
	Colour = 0x505040_rgb;
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

	Weight = 100;

	HeatConduct = 251;
	Description = "NTC Thermistor. Conducts with PSCN and NSCN, but only when heated above 100C.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1687.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_NTCT_update;
}

int Element_NTCT_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp>295.0f)
		parts[i].temp -= 2.5f;
	return 0;
}
