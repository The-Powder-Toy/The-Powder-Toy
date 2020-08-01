#include "simulation/ElementCommon.h"
static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CLNT()
{
	Identifier = "DEFAULT_PT_CLNT";
	Name = "CLNT";
	Colour = PIXPACK(0x00FF77);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "Coolant for reactors and Engines. Use .tmp to set cooling.";

	Properties = TYPE_LIQUID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 8273.0f;
	HighTemperatureTransition = PT_WATR;
	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	{

		if (parts[i].temp - 273.15f >= parts[i].tmp)
		{
			parts[i].temp -= 3.15;
		}
		if (parts[i].temp - 273.15f < parts[i].tmp)
		{
			parts[i].temp += 3.15f;
		}
		return 0;
	}
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->temp - 273.15f == cpart->tmp)
	{
		*colg = 200;
		*colr = 0;
		*colb = 0;
	}
	if (cpart->temp - 294.15f > cpart->tmp)
	{
		*colr = 200;
		*colb = 0;
		*colg = 0;

	}
	if (cpart->temp - 273.15f < cpart->tmp)
	{
		*colg = 0;
		*colb = 200;
		*colr = 0;
	}
	*pixel_mode |= PMODE_FLARE;

	return 0;
}