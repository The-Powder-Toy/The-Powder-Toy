#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ETRD PT_ETRD 50
Element_ETRD::Element_ETRD()
{
	Identifier = "DEFAULT_PT_ETRD";
	Name = "ETRD";
	Colour = PIXPACK(0x404040);
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
	Meltable = 0;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Electrode. Creates a surface that allows Plasma arcs. (Use sparingly)";
	
	State = ST_NONE;
	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;
	
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

Element_ETRD::~Element_ETRD() {}
