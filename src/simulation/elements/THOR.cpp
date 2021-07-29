#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_THOR()
{
	Identifier = "DEFAULT_PT_THOR";
	Name = "THOR";
	Colour = PIXPACK(0xff7700);
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
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x003FC000;

	Weight = 87;

	DefaultProperties.temp = R_TEMP + 30.0f + 273.15f;
	HeatConduct = 251;
	Description = "Thorium. Heavy. Produces heat when struck by Neutrons.";

	Properties = TYPE_PART | PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 273.15f + 1750.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	
	int r, rx, ry, rt;
	for (rx = -1; rx < 2; rx++) {
		for (ry = -1; ry < 2; ry++) {
			if (BOUNDS_CHECK) {
				r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;

				if (TYP(r) == PT_NEUT) {
					// Remove neutron
					sim->kill_part(ID(r));

					// Heat up
					parts[i].temp *= 2.0f;
				}
			}
		}
	}

	return 0;
}
