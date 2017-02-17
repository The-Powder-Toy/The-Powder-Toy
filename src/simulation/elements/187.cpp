#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E187 PT_E187 187
Element_E187::Element_E187()
{
	Identifier = "DEFAULT_PT_E187";
	Name = "E187";
	Colour = PIXPACK(0xFF60D0);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

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
	Description = "Experimental element. like ISOZ.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E187::update;
}

//#TPT-Directive ElementHeader Element_E187 static int update(UPDATE_FUNC_ARGS)
int Element_E187::update(UPDATE_FUNC_ARGS)
{ // for both ISZS and ISOZ
	int r, s;
	float r2, r3;
	if (!(rand()%10000) && !parts[i].tmp)
	{
		s = sim->create_part(-3, x, y, PT_PHOT);
		r2 = (rand()%228+128)/127.0f;
		r3 = (rand()%360)*3.1415926f/180.0f;
		parts[s].vx = r2*cosf(r3);
		parts[s].vy = r2*sinf(r3);
		parts[i].tmp = 1;
		parts[s].tmp = 0x1;
		parts[s].temp = parts[i].temp + 20;
	}
	r = sim->photons[y][x];
	if ((r & 0xFF) == PT_PHOT && !(rand()%100))
	{
		s = sim->create_part(-3, x, y, PT_PHOT);
		r2 = (rand()%228+128)/127.0f;
		r3 = (rand()%360)*3.1415926f/180.0f;
		parts[s].vx = r2*cosf(r3);
		parts[s].vy = r2*sinf(r3);
		parts[s].tmp = 0x1;
		parts[s].temp = parts[i].temp + 20;
	}
	return 0;
}


Element_E187::~Element_E187() {}
