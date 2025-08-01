#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_FIRW()
{
	Identifier = "DEFAULT_PT_FIRW";
	Name = "FIRW";
	Colour = 0xFFA040_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 28;

	Weight = 55;

	HeatConduct = 70;
	Description = "Fireworks! Colorful, set off by fire.";

	Properties = TYPE_PART|PROP_LIFE_DEC;

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
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (parts[i].tmp<=0)
	{
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					auto rt = TYP(r);
					if (rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
					{
						float gx, gy, multiplier;
						sim->GetGravityField(x, y, elements[PT_FIRW].Gravity, 1.0f, gx, gy);
						if (gx*gx+gy*gy < 0.001f)
						{
							float angle = sim->rng.between(0, 6283) * 0.001f;//(in radians, between 0 and 2*pi)
							gx += sinf(angle)*elements[PT_FIRW].Gravity*0.5f;
							gy += cosf(angle)*elements[PT_FIRW].Gravity*0.5f;
						}
						parts[i].tmp = 1;
						parts[i].life = sim->rng.between(20, 29);
						multiplier = (parts[i].life+20)*0.2f/sqrtf(gx*gx+gy*gy);
						parts[i].vx -= gx*multiplier;
						parts[i].vy -= gy*multiplier;
						return 0;
					}
				}
			}
		}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life<=0) {
			parts[i].tmp=2;
		} else {
			parts[i].flags &= ~FLAG_STAGNANT;
		}
	}
	else //if (parts[i].tmp>=2)
	{
		unsigned col = Renderer::firwTableAt(sim->rng.between(0, 199)).Pack();
		for (int n=0; n<40; n++)
		{
			auto np = sim->create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				auto magnitude = sim->rng.between(40, 99) * 0.05f;
				auto angle = sim->rng.between(0, 6283) * 0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx*0.5f + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy*0.5f + sinf(angle)*magnitude;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = sim->rng.between(70, 109);
				parts[np].temp = float(sim->rng.between(5750, 6249));
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		sim->pv[y/CELL][x/CELL] += 8.0f;
		sim->kill_part(i);
		return 1;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp > 0)
	{
		*pixel_mode |= PMODE_GLOW;
	}
	return 0;
}
