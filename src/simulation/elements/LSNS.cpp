#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_LSNS PT_LSNS 185
Element_LSNS::Element_LSNS()
{
	Identifier = "DEFAULT_PT_LSNS";
	Name = "LSNS";
	Colour = PIXPACK(0x336699);
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

	Temperature = 4.0f + 273.15f;
	HeatConduct = 0;
	Description = "Life sensor, creates a spark when there's a nearby particle with a life higher than its temperature.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_LSNS::update;
}

//#TPT-Directive ElementHeader Element_LSNS static int update(UPDATE_FUNC_ARGS)
int Element_LSNS::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, rd = parts[i].tmp2;
	if (rd > 25) parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
		for (rx = -2; rx < 3; rx++)
			for (ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y + ry][x + rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (sim->parts_avg(i, ID(r), PT_INSL) != PT_INSL)
					{
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR) && parts[ID(r)].life == 0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
						}
					}

				}
	}
	bool setFilt = false;
	bool Setnewlife = false;
	int partlife = 0;
	int Newlife = 0;
	for (rx = -rd; rx < rd + 1; rx++)
		for (ry = -rd; ry < rd + 1; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				if (parts[i].tmp == 1 && TYP(r) != PT_LSNS && TYP(r) != PT_FILT && parts[ID(r)].life >= 0)
				{
					setFilt = true;
					partlife = parts[ID(r)].life;
				}
				if (parts[i].tmp == 3 && TYP(r) == PT_FILT)
				{
					Setnewlife = true;
					Newlife = parts[ID(r)].ctype;
				}
				if ((parts[i].tmp != 1) && (parts[i].tmp != 2) && (parts[i].tmp != 3))
				{
					if (parts[ID(r)].life > parts[i].temp - 273.15)
					{
						parts[i].life = 1;
					}
				}
				//Invert mode.
				if (parts[i].tmp == 2)
				{
					if (parts[ID(r)].life > parts[i].temp - 273.15)
						parts[i].life = 0;
					else
						parts[i].life = 1;

				}

			}
	int nx, ny;
	for (int rx = -1; rx <= 1; rx++)
		for (int ry = -1; ry <= 1; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				nx = x + rx;
				ny = y + ry;
				//.life serialization.
				if (setFilt)
				{
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + partlife;
						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
				//.life deserialization.
				if (Setnewlife)
				{
					if (TYP(r) != PT_FILT)
					{
						parts[ID(r)].life = Newlife - 0x10000000;
						break;
					}
				}
			}

	return 0;

}


Element_LSNS::~Element_LSNS() {}