#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_VLSN PT_VLSN 203
Element_VLSN::Element_VLSN()
{
	Identifier = "DEFAULT_PT_VLSN";
	Name = "VLSN";
	Colour = PIXPACK(0x006400);
	MenuVisible = 1;
	MenuSection = SC_New;
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
	Description = "Velocity sensor, creates a spark when there's a nearby particle with a Velocity higher than it's Temp.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_VLSN::update;
	Graphics = &Element_VLSN::graphics;
}

//#TPT-Directive ElementHeader Element_VLSN static int update(UPDATE_FUNC_ARGS)
int Element_VLSN::update(UPDATE_FUNC_ARGS)
{
	int rd = parts[i].tmp2;
	if (rd > 25) parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
		for (int rx = -2; rx <= 2; rx++)
			for (int ry = -2; ry <= 2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y + ry][x + rx];
					if (!r)
						continue;
					int rt = TYP(r);
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
	bool doSerialization = false;
	bool doDeserialization = false;
	int vel = 0;
	for (int rx = -rd; rx < rd + 1; rx++)
		for (int ry = -rd; ry < rd + 1; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;

				switch (parts[i].tmp)
				{
				case 1:
					// .Velocity serialization into FILT
					if (TYP(r) != PT_VLSN && TYP(r) != PT_FILT)
					{
						doSerialization = true;
						vel = std::abs(parts[ID(r)].vx) + std::abs(parts[ID(r)].vy);
					}
					break;
				case 3:
					// .Velocity deserialization
					if (TYP(r) == PT_FILT)
					{
						doDeserialization = true;
						vel = parts[ID(r)].ctype;
					}
					break;
				case 2:
					// Invert mode
					if (TYP(r) != PT_VLSN && TYP(r) != PT_METL)
					{
						if (std::abs(parts[ID(r)].vx) < parts[i].temp - 273.15f && std::abs(parts[ID(r)].vx) > 0)
							parts[i].life = 1;
						if (std::abs(parts[ID(r)].vx) < parts[i].temp - 273.15f && std::abs(parts[ID(r)].vx) > 0)
							parts[i].life = 1;
					}
					break;
				default:
					// Normal mode
					if (TYP(r) != PT_VLSN && TYP(r) != PT_METL)
					{
						if (std::abs(parts[ID(r)].vx) >= parts[i].temp - 273.15f && std::abs(parts[ID(r)].vx) > 0)
							parts[i].life = 1;
						if (std::abs(parts[ID(r)].vx) >= parts[i].temp - 273.15f && std::abs(parts[ID(r)].vx) > 0)
							parts[i].life = 1;
					}
					break;
				}
			}

	for (int rx = -1; rx <= 1; rx++)
		for (int ry = -1; ry <= 1; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				int nx = x + rx;
				int ny = y + ry;
				// .Velocity serialization.
				if (doSerialization)
				{
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + vel;
						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
				// .Velocity deserialization.
				if (doDeserialization)
				{
					if (TYP(r) != PT_FILT)
					{
						parts[ID(r)].vx = vel - 0x10000000;
						parts[ID(r)].vy = vel - 0x10000000;
						break;
					}
				}
			}
	return 0;
}
	//#TPT-Directive ElementHeader Element_VLSN static int graphics(GRAPHICS_FUNC_ARGS)
	int Element_VLSN::graphics(GRAPHICS_FUNC_ARGS)
	{
		if (cpart->life == 1)
		{
			*colg = 255;
			*colr = 0;
			*colb = 0;

		}
		return 0;
	}

Element_VLSN::~Element_VLSN() {}
