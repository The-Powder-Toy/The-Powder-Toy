#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_GSNS()
{
	Identifier = "DEFAULT_PT_GSNS";
	Name = "GSNS";
	Colour = PIXPACK(0x420075);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = 4.0f + 273.15f;
	HeatConduct = 0;
	Description = "Gravity sensor, creates a spark when the nearby gravity is higher than its temperature.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	if ((parts[i].tmp == 0 && (sim->gravp[(y / CELL)*(XRES / CELL) + (x / CELL)] >= parts[i].temp - 273.15f) || (parts[i].tmp == 2 &&  (sim->gravp[(y / CELL)*(XRES / CELL) + (x / CELL)]) < parts[i].temp - 273.15f)))
	{
		parts[i].life = 0;
		for (rx = -2; rx <= 2; rx++)
			for (ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y + ry][x + rx];
					if (!r)
						continue;
					if (sim->parts_avg(i, ID(r), PT_INSL) != PT_INSL)
					{
						rt = TYP(r);
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR) && parts[ID(r)].life == 0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
						}
					}
				}
	}
	for (int rx = -1; rx <= 1; rx++)
		for (int ry = -1; ry <= 1; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				if (parts[i].tmp == 1) //Serialisation 
				{
					float photonWl = sim->gravp[(y / CELL)*(XRES / CELL) + (x / CELL)];
					int nx, ny;
					nx = x + rx;
					ny = y + ry;
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + (signed int)(round(photonWl));
						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
				else if (parts[i].tmp == 3) //Deserialisation 
				{
					if (parts[ID(r)].type == PT_FILT)
					{
						sim->gravmap[(y / CELL)*(XRES / CELL) + (x / CELL)] = (float)(parts[ID(r)].ctype - 0x10000000);
					}
				}
			}
	return 0;
}
