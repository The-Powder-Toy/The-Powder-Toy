#include "simulation/Elements.h"
#include "Sound.h"
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
    Properties = TYPE_SOLID|PROP_LIFE_DEC;

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
	int r,rx,ry;
	parts[i].tmp2 = (int)((parts[i].temp-73.15f)/100+1);
	if(!parts[i].tmp)
		parts[i].tmp=10;
	if(!parts[i].life)
		for(ry=-2;ry<3;ry++)
			for(rx=-2;rx<3;rx++)
				if(BOUNDS_CHECK)
				{
					r = pmap[y+ry][x+rx];
					if(!r)
						continue;
					if((r&0xFF)==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3)
					{
						parts[i].life=2;
						add_note(pow(1.05946309f,parts[i].tmp2)*55,parts[i].tmp*2205);
					}
				}
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
