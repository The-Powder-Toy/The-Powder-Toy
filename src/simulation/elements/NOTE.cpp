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

    Temperature = R_TEMP+4999.0f;
    HeatConduct = 0;
    Description = "Note, plays sound when sparked, tone determined by temperature";

    State = ST_SOLID;
    Properties = TYPE_SOLID;

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
	int r, rx, ry;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
                if ((r&0xFF)==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3)
					add_note(pow(1.05946309,parts[i].tmp)*55);
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_NOTE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_NOTE::graphics(GRAPHICS_FUNC_ARGS)

{
	int q = cpart->tmp*255/100;
	*colr = 0x66;
	*colg = fmin(255,q*2);
	*colb = fmax(0,255-q*2);
	return 0;
}


Element_NOTE::~Element_NOTE() {}
