#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FRNL PT_FRNL 180
Element_FRNL::Element_FRNL()
{
	Identifier = "DEFAULT_PT_FRNL";
	Name = "FRNL";
	Colour = PIXPACK(0x6E8B41);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 3;
	Explosive = 0;
	Meltable = 0;
	Hardness = 3;
	
	Weight = 40;
	
	Temperature = R_TEMP+273.15f;
	HeatConduct = 200;
	Description = "Freon liquid. Can be produced from compression of freon gas. State transitions allow controlled heat flow.";
	
	State = ST_LIQUID;
	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
	
	Update = &Element_FRNL::update;
	
}

//#TPT-Directive ElementHeader Element_FRNL static int update(UPDATE_FUNC_ARGS)
int Element_FRNL::update(UPDATE_FUNC_ARGS)
 {
	 if (sim->pv[y/CELL][x/CELL] < 15.0f)
	{
		parts[i].tmp = parts[i].temp - (273.15f - 50.0f); //tmp is the difference
		parts[i].temp = 273.15f - 50.0f; //Newly decompressed freon liquid should start off at -50 C
		sim->part_change_type(i, parts[i].x, parts[i].y, PT_FREN); //Can't use normal transitions due to the above code needing to run first
	}
	return 0;
}

Element_FRNL::~Element_FRNL() {}
