#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_INDE()
{
	Identifier = "DEFAULT_PT_INDE";
	Name = "INDE";
	Colour = PIXPACK(0xCCFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
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

	Weight = 100;

	HeatConduct = 0;
	Description = "Basically indestructible. It also insulates.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

    Update = &update;
    Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
    int r = pmap[y][x];

    if (TYP(r) == PT_INDE2 && r != i) {
        return 0;
    }
    
    if (TYP(r) != PT_INDE) {
        sim->kill_part(ID(r));
    }

    if ((!pmap[y][x]) || TYP(r) == PT_INDE) {
        sim->create_part(-3, x, y, PT_INDE2);
    }

    return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->create_part(-3, x, y, PT_INDE2);
}
