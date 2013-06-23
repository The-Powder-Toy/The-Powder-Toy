#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LIND PT_LIND 192
Element_LIND::Element_LIND()
{
	Identifier = "DEFAULT_PT_LIND";
	Name = "LIND";
	Colour = PIXPACK(0xCCFFFF);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.10f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = 10;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "General Liquid. Indestructible.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = NULL;
	
}

Element_LIND::~Element_LIND() {}