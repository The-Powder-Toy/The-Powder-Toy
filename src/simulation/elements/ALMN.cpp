#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ALMN PT_ALMN 183
Element_ALMN::Element_ALMN()
{
	Identifier = "DEFAULT_PT_ALMN";
	Name = "ALMN";
	Colour = PIXPACK(0xD2CDCD);
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
	Meltable = 1;
	Hardness = 1;
	
	Weight = 60;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Aluminum, Lower melting point than other metals.";
	
	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 932.5f;
	HighTemperatureTransition = PT_MALM;
	
	Update = NULL;
	
}

Element_ALMN::~Element_ALMN() {}
