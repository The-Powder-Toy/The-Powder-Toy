#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_HEAC()
{
	Identifier = "DEFAULT_PT_HEAC";
	Name = "HEAC";
	Colour = PIXPACK(0xCB6351);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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
	Hardness = 0;

	Weight = 100;

	HeatConduct = 251;
	Description = "Rapid heat conductor.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	// can't melt by normal heat conduction, this is used by other elements for special melting behavior
	HighTemperature = 1887.15f;
	HighTemperatureTransition = NT;

	Update = &update;
}

static const auto isInsulator = [](Simulation* a, int b) -> bool {
	return b && (a->elements[TYP(b)].HeatConduct == 0 || (TYP(b) == PT_HSWC && a->parts[ID(b)].life != 10));
};

// If this is used elsewhere (GOLD), it should be moved into Simulation.h
template<class BinaryPredicate>
bool CheckLine(Simulation* sim, int x1, int y1, int x2, int y2, BinaryPredicate func)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy;
	float e, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
		{
			if (func(sim, sim->pmap[x][y])) return true;
		}
		else
		{
			if (func(sim, sim->pmap[y][x])) return true;
		}
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
				{
					if (func(sim, sim->pmap[x][y])) return true;
				}
				else
				{
					if (func(sim, sim->pmap[y][x])) return true;
				}
			}
			e -= 1.0f;
		}
	}
	return false;
}

static int update(UPDATE_FUNC_ARGS)
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
			if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES && !CheckLine(sim, x, y, x+rrx, y+rry, isInsulator))
			{
				r = pmap[y+rry][x+rrx];
				if (r && sim->elements[TYP(r)].HeatConduct > 0 && (TYP(r) != PT_HSWC || parts[ID(r)].life == 10))
				{
					count++;
					tempAgg += parts[ID(r)].temp;
				}
				r = sim->photons[y+rry][x+rrx];
				if (r && sim->elements[TYP(r)].HeatConduct > 0 && (TYP(r) != PT_HSWC || parts[ID(r)].life == 10))
				{
					count++;
					tempAgg += parts[ID(r)].temp;
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
				if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES && !CheckLine(sim, x, y, x+rrx, y+rry, isInsulator))
				{
					r = pmap[y+rry][x+rrx];
					if (r && sim->elements[TYP(r)].HeatConduct > 0 && (TYP(r) != PT_HSWC || parts[ID(r)].life == 10))
					{
						parts[ID(r)].temp = parts[i].temp;
					}
					r = sim->photons[y+rry][x+rrx];
					if (r && sim->elements[TYP(r)].HeatConduct > 0 && (TYP(r) != PT_HSWC || parts[ID(r)].life == 10))
					{
						parts[ID(r)].temp = parts[i].temp;
					}
				}
			}
		}
	}

	return 0;
}
