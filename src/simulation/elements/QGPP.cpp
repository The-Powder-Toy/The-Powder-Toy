#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_QGPP()
{
	Identifier = "DEFAULT_PT_QGPP";
	Name = "QGP";
	Colour = PIXPACK(0X9400D3);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.40f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 2.0f + 873.15f;
	HeatConduct = 42;
	Description = "Quark Gluon Plasma, a strange element under investigation.";

	Properties = TYPE_GAS | PROP_NEUTPASS;
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 40.0;
	HighPressureTransition = PT_PHOT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	
				if (parts[i].tmp > 0)
				{
					parts[i].tmp--;
					parts[i].tmp2++;
				}

				if (parts[i].tmp == 0 && parts[i].temp >= 474.15f )
				{
					parts[i].tmp = 55;
					parts[i].type = PT_SING;
				}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp <= 400)
	{
		*pixel_mode |= PMODE_LFLARE;
	}
	if (cpart->temp > 273.15f && cpart->temp < 373.15f)
	{
		*colb = 200;
		*colg = 150;
		*colr = 150;
		*pixel_mode |= PMODE_FLARE;
		
	}
	if (cpart->temp >= 374.15f)
	{
		*colb = cpart->tmp2 / 5;
		*colg = cpart->tmp2 / 5;
		*colr = 210;
		*pixel_mode |= PMODE_FLARE;
	}

	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp = 900;
}
