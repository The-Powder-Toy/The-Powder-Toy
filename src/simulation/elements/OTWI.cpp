#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_OTWI PT_OTWI 207
Element_OTWI::Element_OTWI()
{
  Identifier = "DEFAULT_PT_OTWI";
	Name = "OTWI";
	Colour = PIXPACK(0x158A67);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Meltable = 1;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "One time wire, turns into BREL after it has been sparked.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID | PROP_LIFE_DEC | PROP_CONDUCTS | PROP_LIFE_KILL_DEC | PROP_HOT_GLOW;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = PT_LAVA;
	
	Update = Element_OTWI::update;
	
}

//#TPT-Directive ElementHeader Element_OTWI static int update(UPDATE_FUNC_ARGS)
int Element_OTWI::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life==1){sim->part_change_type(i, x, y, PT_BREC); }
	return 0;
}
Element_OTWI::~Element_OTWI() {}
