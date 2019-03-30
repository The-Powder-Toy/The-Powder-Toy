#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SGLU PT_SGLU 191
Element_SGLU::Element_SGLU()
{
	Identifier = "DEFAULT_PT_SGLU";
	Name = "SGLU";
	Colour = PIXPACK(0x445466);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 20;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 72;
	Description = "SOLIDIFIED GLUE,Turns back into glue when extremely cold!";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperature = 273.0f;
	LowTemperatureTransition = PT_GLUE;
	HighTemperatureTransition = NT;

	Update = NULL;
}

Element_SGLU::~Element_SGLU() {}
