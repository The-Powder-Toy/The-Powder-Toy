#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_HEAC PT_HEAC 180
Element_HEAC::Element_HEAC()
{
	Identifier = "DEFAULT_PT_HEAC";
	Name = "HEAC";
	Colour = PIXPACK(0xCB6351);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 50;

	Weight = 100;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 251;
	Description = "Rapid heat conductor.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_HEAC::update;
}

//#TPT-Directive ElementHeader Element_HEAC static int update(UPDATE_FUNC_ARGS)
int Element_HEAC::update(UPDATE_FUNC_ARGS)
{
	const int rad = 4;
	int rry, rrx, r, count = 0;
	float tempAgg = 0;
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			rry = ry * rad;
			rrx = rx * rad;
			if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES)
			{
				r = pmap[y+rry][x+rrx];
				if (r && sim->elements[r&0xFF].HeatConduct > 0)
				{
					count++;
					tempAgg += parts[r>>8].temp;
				}
				r = sim->photons[y+rry][x+rrx];
				if (r && sim->elements[r&0xFF].HeatConduct > 0)
				{
					count++;
					tempAgg += parts[r>>8].temp;
				}
			}
		}
	}

	if (count > 0)
	{
		parts[i].temp = tempAgg/count;

		for (int rx = -1; rx <= 1; rx++)
		{
			for (int ry = -1; ry <= 1; ry++)
			{
				rry = ry * rad;
				rrx = rx * rad;
				if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES)
				{
					r = pmap[y+rry][x+rrx];
					if (r && sim->elements[r&0xFF].HeatConduct > 0)
					{
						parts[r>>8].temp = parts[i].temp;
					}
					r = sim->photons[y+rry][x+rrx];
					if (r && sim->elements[r&0xFF].HeatConduct > 0)
					{
						parts[r>>8].temp = parts[i].temp;
					}
				}
			}
		}
	}

	return 0;
}


Element_HEAC::~Element_HEAC() {}
