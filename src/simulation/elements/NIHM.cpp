#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_NIHM()
{
	Identifier = "DEFAULT_PT_NIHM";
	Name = "NIHM";
	Colour = PIXPACK(0xc94a0a);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.6f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;
	PhotonReflectWavelengths = 0x003FC000;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 30.0f + 273.15f;
	HeatConduct = 255;
	Description = "Molten Nihonium. Extremely unstable. Generates heat.";

	Properties = TYPE_LIQUID | PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	parts[i].temp *= 1.15f;

	if (RNG::Ref().chance(1, 100) && RNG::Ref().chance(int(5.0f * sim->pv[y / CELL][x / CELL]), 100))
	{
		sim->create_part(i, x, y, PT_NEUT);
		sim->create_part(i, x + 1, y, PT_NEUT);
		sim->create_part(i, x - 1, y, PT_NEUT);
		sim->create_part(i, x, y + 1, PT_NEUT);
		sim->create_part(i, x, y - 1, PT_NEUT);
	}

	return 0;
}
