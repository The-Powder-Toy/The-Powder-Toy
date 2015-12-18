#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WOOD PT_WOOD 17
Element_WOOD::Element_WOOD()
{
	Identifier = "DEFAULT_PT_WOOD";
	Name = "WOOD";
	Colour = PIXPACK(0xC0A040);
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

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 15;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 164;
	Description = "Wood, flammable.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 873.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &Element_WOOD::update;
	Graphics = &Element_WOOD::graphics;
}

//#TPT-Directive ElementHeader Element_WOOD static int update(UPDATE_FUNC_ARGS)
int Element_WOOD::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 450 && parts[i].temp > parts[i].tmp)
		parts[i].tmp = (int)parts[i].temp;
	return 0;
}
//#TPT-Directive ElementHeader Element_WOOD static int graphics(GRAPHICS_FUNC_ARGS)
int Element_WOOD::graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = std::max((float)cpart->tmp, cpart->temp);
	if (maxtemp > 400)
	{
		*colr -= (int)restrict_flt((maxtemp-400)/3,0,172);
		*colg -= (int)restrict_flt((maxtemp-400)/4,0,140);
		*colb -= (int)restrict_flt((maxtemp-400)/20,0,44);
	}
	if (maxtemp < 273)
	{
		*colr -= (int)restrict_flt((273-maxtemp)/5,0,40);
		*colg += (int)restrict_flt((273-maxtemp)/4,0,40);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,150);
	}
	return 0;
}

Element_WOOD::~Element_WOOD() {}
