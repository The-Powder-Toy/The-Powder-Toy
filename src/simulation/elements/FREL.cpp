#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FREL PT_FREL 180
Element_FREL::Element_FREL()
{
	Identifier = "PT_FREL";
	Name = "FREL";
	Colour = PIXPACK(0x00ace6);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.9f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 4;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 40;

	Weight = 1;

	Temperature = R_TEMP-80.0f	+273.15f;
	HeatConduct = 200;
	Description = "Liquid Freon. Heats/Cools with temperature change.";

	Properties = TYPE_LIQUID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FREL::update;
}
//#TPT-Directive ElementHeader Element_FREL static int update(UPDATE_FUNC_ARGS)
int Element_FREL::update(UPDATE_FUNC_ARGS)
{
	int pres;
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y / CELL][x / CELL];
	int diff = parts[i].pavg[1] - parts[i].pavg[0];
	diff = (diff * 10);
	parts[i].temp += diff;
	if (parts[i].temp > parts[i].pavg[1] * 6 + 235)
	{
		sim->part_change_type(i, x, y, PT_FREN);
	}
	return 0;
}
Element_FREL::~Element_FREL() {}