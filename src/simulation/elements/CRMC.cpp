#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CRMC PT_CRMC 179
Element_CRMC::Element_CRMC()
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

	Temperature = R_TEMP+273.15f;
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

	Update = &Element_CRMC::update;
	Graphics = &Element_CRMC::graphics;
}

//#TPT-Directive ElementHeader Element_CRMC static int update(UPDATE_FUNC_ARGS)
int Element_CRMC::update(UPDATE_FUNC_ARGS)
{
	if (sim->pv[y/CELL][x/CELL] < -30.0f)
		sim->create_part(i, x, y, PT_CLST);
	return 0;
}

//#TPT-Directive ElementHeader Element_CRMC static int graphics(GRAPHICS_FUNC_ARGS)
int Element_CRMC::graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 2) * 8;
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}

Element_CRMC::~Element_CRMC() {}

