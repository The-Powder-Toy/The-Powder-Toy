#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SPLU()
{
	Identifier = "DEFAULT_PT_SPLU";
	Name = "SPLU";
	Colour = PIXPACK(0x407999);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
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
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x001FCE00;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 4.0f + 273.15f;
	HeatConduct = 251;
	Description = "Solid Plutonium. Heavy, fissile particles. Generates neutrons under pressure.";

	Properties = TYPE_SOLID|PROP_NEUTPASS|PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 245.9;
	HighPressureTransition = PT_PLUT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (RNG::Ref().chance(1, 100) && RNG::Ref().chance(int(5.0f*sim->pv[y/CELL][x/CELL]), 1000))
	{
		sim->create_part(i, x, y, PT_NEUT);
	}
	return 0;
}
