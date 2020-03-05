#include "simulation/ElementCommon.h"

int Element_QRTZ_update(UPDATE_FUNC_ARGS);
int Element_QRTZ_graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_PQRT()
{
	Identifier = "DEFAULT_PT_PQRT";
	Name = "PQRT";
	Colour = PIXPACK(0x88BBBB);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.27f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	HeatConduct = 3;
	Description = "Powdered quartz, broken form of QRTZ.";

	Properties = TYPE_PART| PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2573.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_QRTZ_update;
	Graphics = &Element_QRTZ_graphics;
	Create = &create;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 10);
}
