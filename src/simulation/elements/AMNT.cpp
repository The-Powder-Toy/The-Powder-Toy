#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_AMNT PT_AMNT 179
Element_AMNT::Element_AMNT()
{
	Identifier = "DEFAULT_PT_AMNT";
	Name = "AMNT";
	Colour = PIXPACK(0xECEDED);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
    AirDrag = 0.04f * CFDS;
    AirLoss = 0.94f;
    Loss = 0.95f;
    Collision = -0.1f;
    Gravity = 0.3f;
    Diffusion = 0.00f;
    HotAir = 0.000f  * CFDS;
    Falldown = 1;

    Flammable = 20;
    Explosive = 0;
    Meltable = 0;
    Hardness = 6;

    Weight = 73;

    Temperature = R_TEMP+0.0f + 273.15f;
    HeatConduct = 110;
    Description = "Ammonium Nitrate. Reacts with water and lowers its temperature.";

    State = ST_SOLID;
    Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_AMNT::update;

}

//#TPT-Directive ElementHeader Element_AMNT static int update(UPDATE_FUNC_ARGS)
int Element_AMNT::update(UPDATE_FUNC_ARGS)
 {
    int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

                if ((r&0xFF)==PT_WATR && parts[i].temp > 275.15f)
				{
					parts[i].temp -= 0.1;
				}

				if ((r&0xFF)==PT_DESL)
				{
					parts[i].temp += 0.4;
				}
			}

	return 0;
}

Element_AMNT::~Element_AMNT() {}

