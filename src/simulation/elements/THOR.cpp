#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_THOR()
{
	Identifier = "DEFAULT_PT_THOR";
	Name = "THOR";
	Colour = PIXPACK(0x728d91);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x001FCE00;

	Weight = 90;

	DefaultProperties.temp = R_TEMP + 4.0f + 273.15f;
	HeatConduct = 251;
	Description = "Thorium. Heavy, fissile particles. Reacts to neutrons.";

	Properties = TYPE_PART|PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

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
	if (RNG::Ref().chance(1, 50000) | (parts[i].life % 30 == 1)){
	    sim->create_part(-1, x, y, PT_NEUT);
	    parts[i].temp += 3000;
	}
	return 0;
}
