#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PIND PT_PIND 193
Element_GIND::Element_GIND()
{
	Identifier = "DEFAULT_PT_GIND";
	Name = "GIND";
	Colour = PIXPACK(0xCCFFFF);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;
	
	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.00f;
	Diffusion = 3.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	
	Weight = 10;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 186;
	Description = "General Gas. Indestructible.";
	
	State = ST_GAS;
	Properties = TYPE_GAS;
	
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

Element_GIND::~Element_GIND() {}