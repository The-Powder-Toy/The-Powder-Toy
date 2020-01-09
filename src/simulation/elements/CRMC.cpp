#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_CRMC()
{
	Identifier = "DEFAULT_PT_CRMC";
	Name = "CRMC";
	Colour = PIXPACK(0xD6D1D4);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 35;
	Description = "Ceramic. Gets stronger under pressure.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2887.15f;
	HighTemperatureTransition = ST;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (sim->pv[y/CELL][x/CELL] < -30.0f)
		sim->create_part(i, x, y, PT_CLST);
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 2) * 8;
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = RNG::Ref().between(0, 4);
}
