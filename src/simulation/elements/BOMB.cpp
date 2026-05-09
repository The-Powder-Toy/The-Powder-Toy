#include "simulation/ElementCommon.h"
#include "BOMB.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BOMB()
{
	Identifier = "DEFAULT_PT_BOMB";
	Name = "BOMB";
	Colour = 0xFFF288_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Bomb. Explodes and destroys all surrounding particles when it touches something.";

	Properties = TYPE_PART|PROP_SPARKSETTLE;

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

void Element_BOMB_explode(Simulation* sim, int x, int y)
{
	const int rad = 8;

	for (auto nxj=-rad; nxj<=rad; nxj++)
	{
		for (auto nxi=-rad; nxi<=rad; nxi++)
		{
			if ((nxi*nxi + nxj*nxj) <= rad*rad)
			{
				int ynxj = y+nxj, xnxi = x+nxi;

				if (!InBounds(xnxi, ynxj))
					continue;

				auto r = sim->pmap[ynxj][xnxi];
				auto nt = TYP(r);
				if (nt!=PT_DMND && nt!=PT_CLNE && nt!=PT_PCLN && nt!=PT_BCLN && nt!=PT_VIBR)
				{
					if (nt)
						sim->kill_part(ID(r));
					sim->pv[ynxj/CELL][xnxi/CELL] += 0.1f;
					//@ BOMB -> EMBR
					auto nb = sim->create_part(-3, xnxi, ynxj, PT_EMBR);
					if (nb!=-1)
					{
						sim->parts[nb].tmp = 2;
						sim->parts[nb].life = 2;
						sim->parts[nb].temp = MAX_TEMP;
					}
				}
			}
		}
	}
	for (auto nxj=-(rad+1); nxj<=(rad+1); nxj++)
	{
		for (auto nxi=-(rad+1); nxi<=(rad+1); nxi++)
		{
			if ((nxi*nxi + nxj*nxj) <= (rad+1)*(rad+1))
			{
				int ynxj = y+nxj, xnxi = x+nxi;

				if (!InBounds(xnxi, ynxj))
					continue;

				if (TYP(sim->pmap[ynxj][xnxi]))
					continue;

				//@ BOMB -> EMBR
				auto nb = sim->create_part(-3, xnxi, ynxj, PT_EMBR);
				if (nb!=-1)
				{
					sim->parts[nb].tmp = 0;
					sim->parts[nb].life = 50;
					sim->parts[nb].temp = MAX_TEMP;
					sim->parts[nb].vx = float(sim->rng.between(-20, 20));
					sim->parts[nb].vy = float(sim->rng.between(-20, 20));
				}
			}
		}
	}
}

static int update(UPDATE_FUNC_ARGS)
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
				if (rt!=PT_BOMB && rt!=PT_EMBR && rt!=PT_DMND && rt!=PT_CLNE && rt!=PT_PCLN && rt!=PT_BCLN && rt!=PT_VIBR)
				{
					sim->kill_part(i);
					Element_BOMB_explode(sim, x, y);
					return 1;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}
