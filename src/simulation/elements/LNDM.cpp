#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LNDM PT_LNDM 189
Element_LNDM::Element_LNDM()
{
	Identifier = "DEFAULT_PT_LNDM";
	Name = "LNDM";
	Colour = PIXPACK(0x2B1C1A);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	Temperature = 273.15f;
	HeatConduct = 40;
	Description = "Sensitive Explosive.Sensitive to temp,pressure and stkm!";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 3.0;
	HighPressureTransition = PT_BOMB;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 296.00f;
	HighTemperatureTransition = PT_BOMB;

	Update = &Element_LNDM::update;
}

	//#TPT-Directive ElementHeader Element_LNDM static int update(UPDATE_FUNC_ARGS)
int Element_LNDM::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry,np, rndstore;
	for (rx = -15; rx < 15; rx++)
		for (ry = -15; ry < 15; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_STKM:
					if (RNG::Ref().chance(50, 50))
					{
						parts[i].temp = 350.0f;
					}
				case PT_STKM2:
					if (RNG::Ref().chance(50, 50))
					{
						parts[i].temp = 350.0f;
					}
					
				}
			}
	return 0;
}
Element_LNDM::~Element_LNDM() {}
