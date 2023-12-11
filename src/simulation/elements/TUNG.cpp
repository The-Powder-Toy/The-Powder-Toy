#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_TUNG()
{
	Identifier = "DEFAULT_PT_TUNG";
	Name = "TUNG";
	Colour = 0x505050_rgb;
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "Tungsten. Brittle metal with a very high melting point.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3695.0f;// TUNG melts in its update function instead of in the normal way, but store the threshold here so that it can be changed from Lua
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	bool splode = false;
	const float MELTING_POINT = elements[PT_TUNG].HighTemperature;

	if(parts[i].temp > 2400.0)
	{
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if(TYP(r) == PT_O2)
					{
						splode = true;
					}
				}
			}
		}
	}
	if((parts[i].temp > MELTING_POINT && sim->rng.chance(1, 20)) || splode)
	{
		if (sim->rng.chance(1, 50))
		{
			sim->pv[y/CELL][x/CELL] += 50.0f;
		}
		else if (sim->rng.chance(1, 100))
		{
			sim->part_change_type(i, x, y, PT_FIRE);
			parts[i].life = sim->rng.between(0, 499);
			return 1;
		}
		else
		{
			sim->part_change_type(i, x, y, PT_LAVA);
			parts[i].ctype = PT_TUNG;
			return 1;
		}
		if(splode)
		{
			parts[i].temp = restrict_flt(MELTING_POINT + sim->rng.between(200, 799), MIN_TEMP, MAX_TEMP);
		}
		parts[i].vx += sim->rng.between(-50, 50);
		parts[i].vy += sim->rng.between(-50, 50);
		return 1;
	}
	auto press = int(sim->pv[y/CELL][x/CELL] * 64);
	auto diff = press - parts[i].tmp3;
	if (diff > 32 || diff < -32)
	{
		sim->part_change_type(i,x,y,PT_BRMT);
		parts[i].ctype = PT_TUNG;
		return 1;
	}
	parts[i].tmp3 = press;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	const float MELTING_POINT = elements[PT_TUNG].HighTemperature;
	double startTemp = (MELTING_POINT - 1500.0);
	double tempOver = (((cpart->temp - startTemp)/1500.0)*TPT_PI_FLT) - (TPT_PI_FLT/2.0);
	if(tempOver > -(TPT_PI_FLT/2.0))
	{
		if(tempOver > (TPT_PI_FLT/2.0))
			tempOver = (TPT_PI_FLT/2.0);
		double gradv = sin(tempOver) + 1.0;
		*firer = (int)(gradv * 258.0);
		*fireg = (int)(gradv * 156.0);
		*fireb = (int)(gradv * 112.0);
		*firea = 30;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp3 = int(sim->pv[y/CELL][x/CELL] * 64);
}
