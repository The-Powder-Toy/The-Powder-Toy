#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

// Element overview:
// PAPR (Paper) is a flammable solid element that can be colored by certain other elements.
// Additionally, it can be read and written to by ARAY.

// Property usage:
// life: Written color value. If dcolour is not black, it gets copied here. 1 bit alpha
// tmp: Temporary read/write state for ARAY interaction
// tmp2: Singe level

void Element::Element_PAPR()
{
	Identifier = "DEFAULT_PT_PAPR";
	Name = "PAPR";
	Colour = 0xF3F3CA_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.995f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 15;

	Weight = 100;

	HeatConduct = 80;
	Description = "Paper. Flammable, readable, writable.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 700.0f;
	HighTemperatureTransition = PT_NONE; // Add ash or broken paper element?

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Char when above burning temperature
	if (parts[i].temp > 450 && parts[i].temp >= parts[i].tmp2)
	{
		parts[i].tmp2 = (int)parts[i].temp;
	}

	// Auto-ignition temperature
	if (parts[i].temp > (451.0f - 32.f) / 1.8f + 273.15f)
	{
		parts[i].temp += 1;
		if (sim->rng.chance((int)parts[i].temp-450,400))
		{
			int np = sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_FIRE);
			if (np >= 0)
			{
				parts[np].life = 70;
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Darken when burnt
	float maxtemp = std::max((float)cpart->tmp2, cpart->temp);
	if (maxtemp > 450)
	{
		*colr -= (int)restrict_flt((maxtemp-450)*1.2f,0,230);
		*colg -= (int)restrict_flt((maxtemp-450)*1.4f,0,230);
		*colb -= (int)restrict_flt((maxtemp-450)*1.7f,0,197);
	}
	return 0;
}