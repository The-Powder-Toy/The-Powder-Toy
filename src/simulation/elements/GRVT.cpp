#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GRVT PT_GRVT 177
Element_GRVT::Element_GRVT()
{
	Identifier = "DEFAULT_PT_GRVT";
	Name = "GRVT";
	Colour = PIXPACK(0x00EE76);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = -1;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 61;
	Description = "Gravitons. Create Newtonian Gravity.";

	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_GRVT::update;
	Graphics = &Element_GRVT::graphics;
}

//#TPT-Directive ElementHeader Element_GRVT static int update(UPDATE_FUNC_ARGS)
int Element_GRVT::update(UPDATE_FUNC_ARGS)
{
	//at higher tmps they just go completely insane
	if (parts[i].tmp >= 100)
		parts[i].tmp = 100;
	if (parts[i].tmp <= -100)
		parts[i].tmp = -100;

	sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = 0.2f*parts[i].tmp;
	return 0;
}

//#TPT-Directive ElementHeader Element_GRVT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GRVT::graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 5;
	*firer = 0;
	*fireg = 250;
	*fireb = 170;

	*pixel_mode |= FIRE_BLEND;
	return 1;
}

Element_GRVT::~Element_GRVT() {}
