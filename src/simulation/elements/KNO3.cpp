#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_KNO3 PT_KNO3 179
Element_KNO3::Element_KNO3()
{
	Identifier = "DEFAULT_PT_KNO3";
	Name = "KNO3";
	Colour = PIXPACK(0xCECDCC);
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

    Flammable = 110;
    Explosive = 0;
    Meltable = 0;
    Hardness = 6;

    Weight = 68;

    Temperature = R_TEMP+0.0f + 273.15f;
    HeatConduct = 110;
    Description = "Potassium Nitrate. Good for smoke bombs.";

    State = ST_SOLID;
    Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 340.0f;
	HighTemperatureTransition = PT_SMKE; // Finally, smoke bombs!

	Update = &Element_KNO3::update;

}

//#TPT-Directive ElementHeader Element_KNO3 static int update(UPDATE_FUNC_ARGS)
int Element_KNO3::update(UPDATE_FUNC_ARGS)
 {
    int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				if ((r&0xFF)==PT_SMKE)
				{
					sim->create_part(1, x, y, PT_FIRE);
				}
			}

	return 0;
}

Element_KNO3::~Element_KNO3() {}
