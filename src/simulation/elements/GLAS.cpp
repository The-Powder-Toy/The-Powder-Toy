#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GLAS PT_GLAS 45
Element_GLAS::Element_GLAS()
{
	Identifier = "DEFAULT_PT_GLAS";
	Name = "GLAS";
	Colour = PIXPACK(0x404040);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 150;
	Description = "Glass. Meltable. Shatters under pressure, and refracts photons.";

	Properties = TYPE_SOLID | PROP_NEUTPASS | PROP_HOT_GLOW | PROP_SPARKSETTLE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1973.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_GLAS::update;
}

//#TPT-Directive ElementHeader Element_GLAS static int update(UPDATE_FUNC_ARGS)
int Element_GLAS::update(UPDATE_FUNC_ARGS)
{
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = sim->pv[y/CELL][x/CELL];
	float diff = parts[i].pavg[1] - parts[i].pavg[0];
	if (diff > 0.25f || diff < -0.25f)
	{
		sim->part_change_type(i,x,y,PT_BGLA);
	}
	return 0;
}


Element_GLAS::~Element_GLAS() {}
