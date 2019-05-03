#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_CLNT PT_CLNT 194
Element_CLNT::Element_CLNT()
{
	Identifier = "DEFAULT_PT_CLNT";
	Name = "CLNT";
	Colour = PIXPACK(0x00FF77);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 251;
	Description = "Coolant for reactors and Engines.Turns into WTRV upon excess heating!";

	Properties = TYPE_LIQUID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 6273.0f;
	HighTemperatureTransition = PT_WATR;
	Update = &Element_CLNT::update;
	Graphics = &Element_CLNT::graphics;
}
//#TPT-Directive ElementHeader Element_CLNT static int update(UPDATE_FUNC_ARGS)
int Element_CLNT::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 284.0f)
		parts[i].temp -= 4.5f;
	return 0;
}
//#TPT-Directive ElementHeader Element_CLNT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_CLNT::graphics(GRAPHICS_FUNC_ARGS)

{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}


Element_CLNT::~Element_CLNT() {}
