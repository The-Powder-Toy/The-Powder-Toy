#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PUMP()
{
	Identifier = "DEFAULT_PT_PUMP";
	Name = "PUMP";
	Colour = PIXPACK(0x0A0A3B);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
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

	DefaultProperties.temp = 273.15f;
	HeatConduct = 0;
	Description = "Pressure pump. Changes pressure to its temp when activated. (use HEAT/COOL).";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 10;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life != 10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		if (parts[i].temp >= MAX_PRESSURE+273.15f)
			parts[i].temp = MAX_PRESSURE+273.15f;
		if (parts[i].temp <= MIN_PRESSURE+273.15f)
			parts[i].temp = MIN_PRESSURE+273.15f;

		for (rx = -1; rx <= 1; rx++)
			for (ry = -1; ry <= 1; ry++)
			{
				if (parts[i].tmp != 1)
				{
					if (!(rx && ry))
						sim->pv[(y/CELL)+ry][(x/CELL)+rx] += 0.1f*((parts[i].temp-273.15)-sim->pv[(y/CELL)+ry][(x/CELL)+rx]);
				}
				else
				{
					int r = pmap[y+ry][x+rx];
					if (TYP(r) == PT_FILT)
					{
						int newPressure = parts[ID(r)].ctype - 0x10000000;
						if (newPressure >= 0 && newPressure <= MAX_PRESSURE - MIN_PRESSURE)
						{
							sim->pv[(y + ry) / CELL][(x + rx) / CELL] = float(newPressure) + MIN_PRESSURE;
						}
					}
				}
			}
		for (rx = -2; rx <= 2; rx++)
			for (ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r) == PT_PUMP)
					{
						if (parts[ID(r)].life < 10 && parts[ID(r)].life > 0)
							parts[i].life = 9;
						else if (parts[ID(r)].life == 0)
							parts[ID(r)].life = 10;
					}
				}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colb += lifemod;
	return 0;
}
