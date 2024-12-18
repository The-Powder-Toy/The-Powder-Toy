#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_SING()
{
	Identifier = "DEFAULT_PT_SING";
	Name = "SING";
	Colour = 0x242424_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.36f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.1f;
	Gravity = 0.12f;
	Diffusion = 0.00f;
	HotAir = -0.001f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 86;

	HeatConduct = 70;
	Description = "Singularity. Creates huge amounts of negative pressure and destroys everything.";

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
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	int singularity = -parts[i].life;

	if (sim->pv[y/CELL][x/CELL]<singularity)
		sim->pv[y/CELL][x/CELL] += 0.1f*(singularity-sim->pv[y/CELL][x/CELL]);
	if (sim->pv[y/CELL+1][x/CELL]<singularity)
		sim->pv[y/CELL+1][x/CELL] += 0.1f*(singularity-sim->pv[y/CELL+1][x/CELL]);
	if (sim->pv[y/CELL-1][x/CELL]<singularity)
		sim->pv[y/CELL-1][x/CELL] += 0.1f*(singularity-sim->pv[y/CELL-1][x/CELL]);

	sim->pv[y/CELL][x/CELL+1] += 0.1f*(singularity-sim->pv[y/CELL][x/CELL+1]);
	sim->pv[y/CELL+1][x/CELL+1] += 0.1f*(singularity-sim->pv[y/CELL+1][x/CELL+1]);
	sim->pv[y/CELL][x/CELL-1] += 0.1f*(singularity-sim->pv[y/CELL][x/CELL-1]);
	sim->pv[y/CELL-1][x/CELL-1] += 0.1f*(singularity-sim->pv[y/CELL-1][x/CELL-1]);

	if (parts[i].life<1) {
		//Pop!
		for (auto rx = -1; rx <= 1; rx++)
		{
			auto crx = (x/CELL)+rx;
			for (auto ry = -1; ry <= 1; ry++)
			{
				auto cry = (y/CELL)+ry;
				if (cry >= 0 && crx >= 0 && crx < XCELLS && cry < YCELLS) {
					sim->pv[cry][crx] += (float)parts[i].tmp;
				}
			}
		}
		auto spawncount = std::abs(parts[i].tmp);
		spawncount = (spawncount>255) ? 3019 : int(std::pow((double)(spawncount/8), 2)*TPT_PI_FLT);
		for (int j = 0;j < spawncount; j++)
		{
			auto nb = -1;
			switch (sim->rng.gen() % 3)
			{
				case 0:
					nb = sim->create_part(-3, x, y, PT_PHOT);
					break;
				case 1:
					nb = sim->create_part(-3, x, y, PT_NEUT);
					break;
				case 2:
					nb = sim->create_part(-3, x, y, PT_ELEC);
					break;
			}
			if (nb!=-1) {
				parts[nb].life = sim->rng.between(0, 299);
				parts[nb].temp = MAX_TEMP/2;
				auto angle = sim->rng.uniform01()*2.0f*TPT_PI_FLT;
				auto v = sim->rng.uniform01()*5.0f;
				parts[nb].vx = v*cosf(angle);
				parts[nb].vy = v*sinf(angle);
			}
			else if (sim->MaxPartsReached())
				break;//if we've run out of particles, stop trying to create them - saves a lot of lag on "sing bomb" saves
		}
		sim->kill_part(i);
		return 1;
	}
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_DMND&& sim->rng.chance(1, 3))
				{
					if (TYP(r)==PT_SING && parts[ID(r)].life >10)
					{
						if (parts[i].life+parts[ID(r)].life > 255)
							continue;
						parts[i].life += parts[ID(r)].life;
					}
					else
					{
						if (parts[i].life+3 > 255)
						{
							if (parts[ID(r)].type!=PT_SING && sim->rng.chance(1, 1000))
							{
								int np;
								np = sim->create_part(ID(r),x+rx,y+ry,PT_SING);
								parts[np].life = sim->rng.between(60, 109);
							}
							continue;
						}
						parts[i].life += 3;
						parts[i].tmp++;
					}
					parts[i].temp = restrict_flt(parts[ID(r)].temp+parts[i].temp, MIN_TEMP, MAX_TEMP);
					sim->kill_part(ID(r));
				}
			}
		}
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = sim->rng.between(60, 109);
}
