#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_URAN PT_URAN 32
Element_URAN::Element_URAN()
{
	Identifier = "DEFAULT_PT_URAN";
	Name = "URAN";
	Colour = PIXPACK(0x707020);
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
	PhotonReflectWavelengths = 0x003FC000;

	Weight = 90;

	Temperature = R_TEMP+30.0f+273.15f;
	HeatConduct = 251;
	Description = "Uranium. Heavy particles. Generates heat under pressure.";

	Properties = TYPE_PART | PROP_RADIOACTIVE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_URAN::update;
}

//#TPT-Directive ElementHeader Element_URAN static int update(UPDATE_FUNC_ARGS)
int Element_URAN::update(UPDATE_FUNC_ARGS)
{
	if (!sim->legacy_enable && sim->pv[y/CELL][x/CELL]>0.0f)
	{
		if (parts[i].temp == MIN_TEMP)
		{
			parts[i].temp += .01f;
		}
		else
		{
			parts[i].temp = restrict_flt((parts[i].temp*(1 + (sim->pv[y / CELL][x / CELL] / 2000))) + MIN_TEMP, MIN_TEMP, MAX_TEMP);
		}
	}
	return 0;
}


Element_URAN::~Element_URAN() {}
