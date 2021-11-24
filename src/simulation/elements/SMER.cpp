#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SMER()
{
	Identifier = "DEFAULT_PT_SMER";
	Name = "SMER";
	Colour = PIXPACK(0x736D6E);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.800f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 91;

	DefaultProperties.temp = 234.33f;
	HeatConduct = 251;
	Description = "Solid Mercury. Conductive and absorbs neutrons, turns back to MERC when above -38.83C.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_NEUTABSORB|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITH;
	LowTemperatureTransition = NT;
	HighTemperature = 234.31f;
	HighTemperatureTransition = PT_MERC;

	DefaultProperties.tmp = 10;

	Update = &update;
}
