#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ISZS()
{
	Identifier = "DEFAULT_PT_ISZS";
	Name = "ISZS";
	Colour = PIXPACK(0x662089);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.0007f* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = 140.00f;
	HeatConduct = 251;
	Description = "Solid form of ISOZ, slowly decays into PHOT.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 300.0f;
	HighTemperatureTransition = PT_ISOZ;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	float rr, rrr;
	if (RNG::Ref().chance(1, 200) && RNG::Ref().chance(-4.0f * sim->pv[y/CELL][x/CELL], 1000))
	{
		sim->create_part(i, x, y, PT_PHOT);
		rr = RNG::Ref().between(128, 355) / 127.0f;
		rrr = RNG::Ref().between(0, 359) * 3.14159f / 180.0f;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
	}
	return 0;
}
