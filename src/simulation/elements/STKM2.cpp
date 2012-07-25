#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_STKM2 PT_STKM2 128
Element_STKM2::Element_STKM2()
{
    Identifier = "DEFAULT_PT_STKM2";
    Name = "STK2";
    Colour = PIXPACK(0x000000);
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
    Description = "Stickman. Don't kill him!";
    
    State = ST_NONE;
    Properties = 0;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 620.0f;
    HighTemperatureTransition = PT_FIRE;
    
    Update = &Element_STKM2::update;
    Graphics = &Element_STKM2::graphics;
}

//#TPT-Directive ElementHeader Element_STKM2 static int update(UPDATE_FUNC_ARGS)
int Element_STKM2::update(UPDATE_FUNC_ARGS)
 {
	Element_STKM::run_stickman(&sim->player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}



//#TPT-Directive ElementHeader Element_STKM2 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_STKM2::graphics(GRAPHICS_FUNC_ARGS)

{	
	*pixel_mode = PSPEC_STICKMAN;
	/*if ((int)sim->player2.elem<PT_NUM)
	{
		*colr = PIXR(elements[sim->player2.elem].pcolors);
		*colg = PIXG(elements[sim->player2.elem].pcolors);
		*colb = PIXB(elements[sim->player2.elem].pcolors);
	}
	else*/
	{
		*colr = *colg = *colb = 255;
	}
	return 1;
}


Element_STKM2::~Element_STKM2() {}