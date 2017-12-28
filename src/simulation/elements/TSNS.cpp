#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_TSNS PT_TSNS 164
Element_TSNS::Element_TSNS()
{
	Identifier = "DEFAULT_PT_TSNS";
	Name = "TSNS";
	Colour = PIXPACK(0xFD00D5);
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

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 0;
	Description = "Temperature sensor, creates a spark when there's a nearby particle with a greater temperature.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_TSNS::update;
}

//#TPT-Directive ElementHeader Element_TSNS static int update(UPDATE_FUNC_ARGS)
int Element_TSNS::update(UPDATE_FUNC_ARGS)
{
	int rd = parts[i].tmp2;
	if (rd > 25)
		parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
		for (int rx = -2; rx <= 2; rx++)
			for (int ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					int rt = TYP(r);
					if (sim->parts_avg(i, ID(r), PT_INSL) != PT_INSL)
					{
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR) && parts[ID(r)].life == 0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r), x+rx, y+ry, PT_SPRK);
						}
					}
				}
	}
	bool setFilt = false;
	int photonWl = 0;
	for (int rx = -rd; rx <= rd; rx++)
		for (int ry = -rd; ry <= rd; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r) != PT_TSNS && TYP(r) != PT_METL && parts[ID(r)].temp > parts[i].temp)
					parts[i].life = 1;
				if (parts[i].tmp == 1 && TYP(r) != PT_TSNS && TYP(r) != PT_FILT)
				{
					setFilt = true;
					photonWl = parts[ID(r)].temp;
				}
			}
	if (setFilt)
	{
		int nx, ny;
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					nx = x + rx;
					ny = y + ry;
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + photonWl;
						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
	}
	return 0;
}



Element_TSNS::~Element_TSNS() {}
