#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NOTE PT_NOTE 167
Element_NOTE::Element_NOTE()
{
    Identifier = "DEFAULT_PT_NOTE";
    Name = "NOTE";
    Colour = PIXPACK(0x66AAFF);
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
    Hardness = 2;

    Weight = 100;

    Temperature = 273.15+1046.5;
    HeatConduct = 0;
    Description = "Note, plays sound when sparked, tone determined by temperature, length by tmp";

    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = 15.0f;
    HighPressureTransition = PT_BREC;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;

    Update = &Element_NOTE::update;
    Graphics = &Element_NOTE::graphics;
}

//#TPT-Directive ElementHeader Element_NOTE static int update(UPDATE_FUNC_ARGS)
int Element_NOTE::update(UPDATE_FUNC_ARGS)
{
	if(!parts[i].tmp)
		parts[i].tmp=10;
	parts[i].tmp2 = (int)((parts[i].temp-73.15f)/100+1);
	return 0;
}


//#TPT-Directive ElementHeader Element_NOTE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_NOTE::graphics(GRAPHICS_FUNC_ARGS)

{
	*colr = 0x66;
	*colg = cpart->tmp2*255/100;
	*colb = cpart->tmp*255/100;
	return 0;
}


Element_NOTE::~Element_NOTE() {}
