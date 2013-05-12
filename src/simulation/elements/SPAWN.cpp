#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SPAWN PT_SPAWN 118
Element_SPAWN::Element_SPAWN()
{
	Identifier = "DEFAULT_PT_SPAWN";
	Name = "SPWN";
	Colour = PIXPACK(0xAAAAAA);
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "STKM spawn point.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_SPAWN::update;
	
}

//#TPT-Directive ElementHeader Element_SPAWN static int update(UPDATE_FUNC_ARGS)
int Element_SPAWN::update(UPDATE_FUNC_ARGS)
 {
	if (!sim->player.spwn)
		sim->create_part(-1, x, y, PT_STKM);

	return 0;
}



Element_SPAWN::~Element_SPAWN() {}
