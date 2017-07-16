#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PLUT PT_PLUT 19
Element_PLUT::Element_PLUT()
{
	Identifier = "DEFAULT_PT_PLUT";
	Name = "PLUT";
	Colour = PIXPACK(0x407020);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x001FCE00;

	Weight = 90;

	Temperature = R_TEMP+4.0f	+273.15f;
	HeatConduct = 251;
	Description = "Plutonium. Heavy, fissile particles. Generates neutrons under pressure.";

	Properties = TYPE_PART|PROP_NEUTPASS|PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PLUT::update;
}

//#TPT-Directive ElementHeader Element_PLUT static int update(UPDATE_FUNC_ARGS)
int Element_PLUT::update(UPDATE_FUNC_ARGS)
{
	if (!(rand()%100) && ((int)(5.0f*sim->pv[y/CELL][x/CELL]))>(rand()%1000))
	{
		sim->create_part(i, x, y, PT_NEUT);
	}
	return 0;
}


Element_PLUT::~Element_PLUT() {}
