#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_RFGL PT_RFGL 184
Element_RFGL::Element_RFGL()
{
	Identifier = "DEFAULT_PT_RFGL";
	Name = "RFGL";
	Colour = PIXPACK(0x84C2CF);
	MenuVisible = 0;
	MenuSection = SC_LIQUID;
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
	Hardness = 20;

	Weight = 10;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 0;
	Description = "Liquid refrigerant.";

	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_RFGL::update;
}

//#TPT-Directive ElementHeader Element_RFGL static int update(UPDATE_FUNC_ARGS)
int Element_RFGL::update(UPDATE_FUNC_ARGS)
{
	float pressure = sim->pv[y/CELL][x/CELL];
	if (pressure > parts[i].tmp)
		parts[i].tmp = (int)pressure;

	if (pressure > -1 && pressure < 15 && parts[i].life > 0)
		parts[i].life --;

	if (parts[i].temp >= 363.15f + (pressure * 6.0f))
		sim->part_change_type(i, x, y, PT_RFRG);

	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF) == PT_RFGL || (r&0xFF) == PT_RFRG)
				{
					float avgTemp = (parts[r>>8].temp + parts[i].temp) / 2;
					parts[r>>8].temp = avgTemp;
					parts[i].temp = avgTemp;
				}
				else
				{
					if (pressure > 20 && parts[i].temp > 273.15f + 2.0f - (pressure - 20.0f) && sim->elements[r&0xFF].HeatConduct)
					{
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp + 80.0f, 0.0f, MAX_TEMP);
						parts[i].temp = restrict_flt(parts[i].temp - 80.0f, 0.0f, MAX_TEMP);
					}
					else if (parts[i].life == 0 && parts[r>>8].temp > 273.15f - 50.0f - (parts[i].tmp - 20.0f) && sim->elements[r&0xFF].HeatConduct)
					{
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp - 80.0f, 273.15f - 50.0f - (parts[i].tmp - 20.0f), MAX_TEMP);
						parts[i].temp = restrict_flt(parts[i].temp + 80.0f, 0.0f, 383.15f);
					}
				}
			}

	return 0;
}

Element_RFGL::~Element_RFGL() {}
