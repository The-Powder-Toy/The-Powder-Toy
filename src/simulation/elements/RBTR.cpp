#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RBTR PT_RBTR 192
Element_RBTR::Element_RBTR()
{
	Identifier = "DEFAULT_PT_RBTR";
	Name = "RBTR";
	Colour = PIXPACK(0x8EA8B8);
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

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 260;
	Description = "Realistic battery,PSCN activate,NSCN deactivate.INST charges.Tmp2=1 fast charges.Use sparingly!";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RBTR::update;
}
//#TPT-Directive ElementHeader Element_RBTR static int update(UPDATE_FUNC_ARGS)
int Element_RBTR::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np, rndstore,transfer;
	for (rx = -3; rx < 3; rx++)
		for (ry = -3; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_METL:
					if (parts[i].temp > 274.15f && parts[i].tmp == 10)
					{
						parts[i].temp -= 1;
						np = sim->create_part(ID(r), x + rx, y + ry, PT_SPRK);
					
					}

				}
			}
	return 0;
}
				
				
Element_RBTR::~Element_RBTR() {}
