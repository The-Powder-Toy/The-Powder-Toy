#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);


void Element::Element_VLSN()
{
	Identifier = "DEFAULT_PT_VLSN";
	Name = "VLSN";
	Colour = PIXPACK(0x7CFC00);
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

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
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
	int velx = 0;
	int vely = 0;
	for (int rx = -rd; rx < rd + 1; rx++)
		for (int ry = -rd; ry < rd + 1; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				int velx, vely, velm = 0;
				velx = std::abs(parts[ID(r)].vx);
				vely = std::abs(parts[ID(r)].vy);
				velm = sqrt(velx ^ 2 + vely ^ 2);
				switch (parts[i].tmp)
				{
				case 2:
					// Invert mode
					if (TYP(r) != PT_VLSN && TYP(r) != PT_METL)
					{
						if (velm < parts[i].temp - 273.15f)
							parts[i].life = 1;

					}
					break;
				default:
					// Normal mode
					if (TYP(r) != PT_VLSN && TYP(r) != PT_METL)
					{
						if (velm > parts[i].temp - 273.15f)
							parts[i].life = 1;

					}
					break;
				case 1:
					// .Velocity serialization
					if (TYP(r) != PT_VLSN && TYP(r) != PT_FILT)
					{
						doSerialization = true;
						velx = std::abs(parts[ID(r)].vx);
						vely = std::abs(parts[ID(r)].vy);
					}
					break;
				case 3:
					// .Velocity deserialization
					if (TYP(r) != PT_VLSN && TYP(r) != PT_FILT)
					{
						doDeserialization = true;
						velx = std::abs(parts[ID(r)].vx);
						vely = std::abs(parts[ID(r)].vy);
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
				int velx, vely, velm = 0;
				velx = std::abs(parts[ID(r)].vx);
				vely = std::abs(parts[ID(r)].vy);
				velm = sqrt(velx ^ 2 + vely ^ 2);
				int nx = x + rx;
				int ny = y + ry;
				// .Velocity serialization.
				if (doSerialization)
				{
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + velm;

						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
				// .Velocity deserialization.
				if (doSerialization)
				{
					while (TYP(r) != PT_FILT)
					{
						parts[ID(r)].vx = velm - 0x10000000;
						parts[ID(r)].vy = velm - 0x10000000;
					}
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life == 1)
	{
		*colg = 255;
		*colr = 0;
		*colb = 0;
	}
	else if (cpart->life == 0)
	{
		*colg = 124;
		*colr = 0;
		*colb = 0;
	}
	return 0;
}