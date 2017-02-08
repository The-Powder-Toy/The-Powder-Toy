#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FREN PT_FREN 181
Element_FREN::Element_FREN()
{
	Identifier = "PT_FREN";
	Name = "FREN";
	Colour = PIXPACK(0x00ffff);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 4.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 40;

	Weight = 1;

	Temperature = R_TEMP-40.0f	+273.15f;
	HeatConduct = 200;
	Description = "Freon Gas. Heats/Cools with temperature change.";

	Properties = TYPE_GAS | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FREN::update;
}
//#TPT-Directive ElementHeader Element_FREN static int update(UPDATE_FUNC_ARGS)
int Element_FREN::update(UPDATE_FUNC_ARGS)
{
	int pres;
	float diff;
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y / CELL][x / CELL];
	diff = parts[i].pavg[1] - parts[i].pavg[0];
	diff = (diff * 10.0f);
	parts[i].temp += diff;
	if (parts[i].temp < parts[i].pavg[1] * 6 + 235)
	{
		sim->part_change_type(i, x, y, PT_FREL);
	}
	return 0;
}
Element_FREN::~Element_FREN() {}