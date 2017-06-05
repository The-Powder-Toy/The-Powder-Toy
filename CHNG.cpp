#include "simulation/Elements.h"
Element_CHNG::Element_CHNG()
{
	Identifier = "DEFAULT_PT_CHNG";
	Name = "CHNG";
	Colour = PIXPACK(0xff6600);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
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
	HeatConduct = 0;
	Description = "Change element. Changes into its ctype when life is 0.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_DRAWONCTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_CHNG::update;
}
int Element_CHNG::update(UPDATE_FUNC_ARGS)
{
	if(parts[i].life==0)
        {
	  sim->part_change_type(i,x,y,parts[i].ctype);
        }
	return 0;
}

Element_CHNG::~Element_CHNG() {}
