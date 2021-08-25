#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_NIH()
{
	Identifier = "DEFAULT_PT_NIH";
	Name = "NIH";
	Colour = PIXPACK(0xc94a0a);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.6f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;
	PhotonReflectWavelengths = 0x003FC000;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 30.0f + 273.15f;
	HeatConduct = 255;
	Description = "Nihonium. Extremely unstable. Generates heat.";

	Properties = TYPE_PART | PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 273.15f + 1750.0f;
	HighTemperatureTransition = PT_NIHM;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	parts[i].temp *= 1.1f;

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
