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
	
	State = ST_NONE;
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
	Create = &Element_STKM2::create;
}

//#TPT-Directive ElementHeader Element_STKM2 static int update(UPDATE_FUNC_ARGS)
int Element_STKM2::update(UPDATE_FUNC_ARGS)
 {
	Element_STKM::run_stickman(&sim->player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

//#TPT-Directive ElementHeader Element_STKM2 static void create(CREATE_FUNC_ARGS)
void Element_STKM2::create(CREATE_FUNC_ARGS)
{
	if (sim->player2.spwn==0)
	{
		parts[i].life = 100;
		Element_STKM::STKM_init_legs(sim, &sim->player2, i);
		sim->player2.spwn = 1;
		sim->player2.rocketBoots = false;
	}
	else
	{
		parts[i].type=0;
	}
	sim->create_part(-3,x,y,PT_SPAWN2);
}

Element_STKM2::~Element_STKM2() {}
