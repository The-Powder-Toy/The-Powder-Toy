#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_STKM2 PT_STKM2 128
Element_STKM2::Element_STKM2()
{
	Identifier = "DEFAULT_PT_STKM2";
	Name = "STK2";
	Colour = PIXPACK(0x6464FF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.5f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.2f;
	Loss = 1.0f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.00f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 50;

	Temperature = R_TEMP+14.6f+273.15f;
	HeatConduct = 0;
	Description = "Second stickman. Don't kill him! Control with wasd.";

	Properties = PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 620.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &Element_STKM2::update;
	Graphics = &Element_STKM::graphics;
}

//#TPT-Directive ElementHeader Element_STKM2 static int update(UPDATE_FUNC_ARGS)
int Element_STKM2::update(UPDATE_FUNC_ARGS)
{
	Element_STKM::run_stickman(&sim->player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

Element_STKM2::~Element_STKM2() {}
