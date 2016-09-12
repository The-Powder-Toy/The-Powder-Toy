#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RFRG PT_RFRG 183
Element_RFRG::Element_RFRG()
{
	Identifier = "DEFAULT_PT_RFRG";
	Name = "RFRG";
	Colour = PIXPACK(0x72D2D4);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.2f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 1.3f;
	HotAir = 0.0001f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 1;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 3;
	Description = "Refrigerant. Liquifies and transfers heat to other particles under pressure.";

	Properties = TYPE_GAS|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RFRG::update;
}

//#TPT-Directive ElementHeader Element_RFRG static int update(UPDATE_FUNC_ARGS)
int Element_RFRG::update(UPDATE_FUNC_ARGS)
{
	if (sim->pv[y/CELL][x/CELL] > 15)
	{
		parts[i].temp += (sim->pv[y/CELL][x/CELL] - 15.0f) / 2.0f;
		if (parts[i].temp >= 343.15f)
		{
			sim->part_change_type(i, x, y, PT_RFGL);
			parts[i].life = 20;
		}
	}

	return 0;
}


Element_RFRG::~Element_RFRG() {}
