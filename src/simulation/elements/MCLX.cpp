#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_MCLX PT_MCLX 186
Element_MCLX::Element_MCLX()
{
	Identifier = "DEFAULT_PT_MCLX";
	Name = "MCLX";
	Colour = PIXPACK(0xFFFDD0);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 2;
	Description = "Insulator, blocks electricity, does not burn.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1144.261;
	HighTemperatureTransition = PT_LAVA;
	
	Update = NULL;
	
}

Element_MCLX::~Element_MCLX() {}
