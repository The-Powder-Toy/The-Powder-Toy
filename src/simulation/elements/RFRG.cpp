#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RFRG PT_RFRG 183
Element_RFRG::Element_RFRG()
{
	Identifier = "DEFAULT_PT_RFRG";
	Name = "RFRG";
	Colour = PIXPACK(0x72D2D4);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.2f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 1.3f;
	HotAir = 0.0001f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 1;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 3;
	Description = "Refrigerant. Heats up and liquefies under pressure.";

	Properties = TYPE_GAS|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 2;
	HighPressureTransition = PT_RFGL;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RFRG::update;
}

//#TPT-Directive ElementHeader Element_RFRG static int update(UPDATE_FUNC_ARGS)
int Element_RFRG::update(UPDATE_FUNC_ARGS)
{
	float new_pressure = sim->pv[y/CELL][x/CELL];
	float *old_pressure = (float *)&parts[i].tmp;
	if (std::isnan(*old_pressure))
	{
		*old_pressure = new_pressure;
		return 0;
	}
	
	// * 0 bar seems to be pressure value -256 in TPT, see Air.cpp. Also, 1 bar seems to be pressure value 0.
	//   With those two values we can set up our pressure scale which states that ... the highest pressure
	//   we can achieve in TPT is 2 bar. That's not particularly realistic, but good enough for TPT.
	
	parts[i].temp = restrict_flt(parts[i].temp * ((new_pressure + 257.f) / (*old_pressure + 257.f)), 0, MAX_TEMP);
	*old_pressure = new_pressure;
	return 0;
}


Element_RFRG::~Element_RFRG() {}
