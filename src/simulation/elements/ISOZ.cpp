#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ISOZ PT_ISOZ 107
Element_ISOZ::Element_ISOZ()
{
	Identifier = "DEFAULT_PT_ISOZ";
	Name = "ISOZ";
	Colour = PIXPACK(0xAA30D0);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
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

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 24;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 29;
	Description = "Isotope-Z. Radioactive liquid, decays into photons when touching PHOT or under negative pressure.";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 160.0f;
	LowTemperatureTransition = PT_ISZS;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_ISOZ::update;
}

//#TPT-Directive ElementHeader Element_ISOZ static int update(UPDATE_FUNC_ARGS)
int Element_ISOZ::update(UPDATE_FUNC_ARGS)
 { // for both ISZS and ISOZ
	float rr, rrr;
	if (!(rand()%200) && ((int)(-4.0f*(sim->pv[y/CELL][x/CELL])))>(rand()%1000))
	{
		sim->create_part(i, x, y, PT_PHOT);
		rr = (rand()%228+128)/127.0f;
		rrr = (rand()%360)*3.14159f/180.0f;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
	}
	return 0;
}


Element_ISOZ::~Element_ISOZ() {}
