#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SING PT_SING 131
Element_SING::Element_SING()
{
	Identifier = "DEFAULT_PT_SING";
	Name = "SING";
	Colour = PIXPACK(0x242424);
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

	Temperature = R_TEMP+0.0f	+273.15f;
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

	Update = &Element_SING::update;
}

//#TPT-Directive ElementHeader Element_SING static int update(UPDATE_FUNC_ARGS)
int Element_SING::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, cry, crx, nb, spawncount;
	int singularity = -parts[i].life;
	float angle, v;

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
		for (rx=-1; rx<2; rx++) {
			crx = (x/CELL)+rx;
			for (ry=-1; ry<2; ry++) {
				cry = (y/CELL)+ry;
				if (cry >= 0 && crx >= 0 && crx < (XRES/CELL) && cry < (YRES/CELL)) {
					sim->pv[cry][crx] += (float)parts[i].tmp;
				}
			}
		}
		spawncount = std::abs(parts[i].tmp);
		spawncount = (spawncount>255) ? 3019 : std::pow((double)(spawncount/8), 2)*M_PI;
		for (int j = 0;j < spawncount; j++)
		{
			switch(rand()%3)
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
				parts[nb].life = (rand()%300);
				parts[nb].temp = MAX_TEMP/2;
				angle = rand()*2.0f*M_PI/RAND_MAX;
				v = (float)(rand())*5.0f/RAND_MAX;
				parts[nb].vx = v*cosf(angle);
				parts[nb].vy = v*sinf(angle);
			}
			else if (sim->pfree==-1)
				break;//if we've run out of particles, stop trying to create them - saves a lot of lag on "sing bomb" saves
		}
		sim->kill_part(i);
		return 1;
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_DMND&& !(rand()%3))
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
							if (parts[ID(r)].type!=PT_SING && !(rand()%100))
							{
								int np;
								np = sim->create_part(ID(r),x+rx,y+ry,PT_SING);
								parts[np].life = rand()%50+60;
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
	return 0;
}


Element_SING::~Element_SING() {}
