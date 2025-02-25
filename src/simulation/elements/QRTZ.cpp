#include "simulation/ElementCommon.h"
#include "QRTZ.h"

static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_QRTZ()
{
	Identifier = "DEFAULT_PT_QRTZ";
	Name = "QRTZ";
	Colour = 0xAADDDD_rgb;
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
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 3;
	Description = "Quartz, breakable mineral. Conducts but becomes brittle when cold. Scatters photons.";

	Properties = TYPE_SOLID | PROP_PHOTPASS | PROP_HOT_GLOW | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2573.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_QRTZ_update;
	Graphics = &Element_QRTZ_graphics;
	Create = &create;
}

int Element_QRTZ_update(UPDATE_FUNC_ARGS)
{
	int t = parts[i].type;
	if (t == PT_QRTZ)
	{
		auto press = int(sim->pv[y/CELL][x/CELL] * 64);
		auto diffTolerance = parts[i].temp * 1.0666f;
		if (press - parts[i].tmp3 > diffTolerance || press - parts[i].tmp3 < -diffTolerance)
		{
			sim->part_change_type(i,x,y,PT_PQRT);
			parts[i].life = 5; //timer before it can grow or diffuse again
		}
		parts[i].tmp3 = press;
	}
	if (parts[i].life>5)
		parts[i].life = 5;
	// absorb SLTW
	if (parts[i].tmp != -1)
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
					else if (TYP(r)==PT_SLTW && sim->rng.chance(1, 500))
					{
						sim->kill_part(ID(r));
						parts[i].tmp++;
					}
				}
			}
		}
	}
	// grow and diffuse
	if (parts[i].tmp > 0 && (parts[i].vx*parts[i].vx + parts[i].vy*parts[i].vy)<0.2f && parts[i].life<=0)
	{
		bool stopgrow = false;
		for (auto trade = 0; trade < 9; trade++)
		{
			auto rnd = sim->rng.gen() % 0x3FF;
			auto rx = (rnd%5)-2;
			auto srx = (rnd%3)-1;
			rnd >>= 3;
			auto ry = (rnd%5)-2;
			auto sry = (rnd%3)-1;
			if (rx || ry)
			{
				if (!stopgrow)//try to grow
				{
					if (!pmap[y+sry][x+srx] && parts[i].tmp!=0)
					{
						auto np = sim->create_part(-1,x+srx,y+sry,PT_QRTZ);
						if (np>-1)
						{
							parts[np].temp = parts[i].temp;
							parts[np].tmp2 = parts[i].tmp2;
							if (sim->rng.chance(1, 2))
							{
								parts[np].tmp2 = std::clamp(parts[np].tmp2 + sim->rng.between(-1, 1), 0, 10);
							}
							parts[i].tmp--;
							if (t == PT_PQRT)
							{
								// If PQRT is stationary and has started growing particles of QRTZ, the PQRT is basically part of a new QRTZ crystal. So turn it back into QRTZ so that it behaves more like part of the crystal.
								sim->part_change_type(i,x,y,PT_QRTZ);
							}
							if (sim->rng.chance(1, 2))
							{
								parts[np].tmp=-1;//dead qrtz
							}
							else if (!parts[i].tmp && sim->rng.chance(1, 15))
							{
								parts[i].tmp=-1;
							}
							stopgrow=true;
						}
					}
				}
				//diffusion
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if (TYP(r)==PT_QRTZ && (parts[i].tmp>parts[ID(r)].tmp) && parts[ID(r)].tmp>=0)
				{
					auto tmp = parts[i].tmp - parts[ID(r)].tmp;
					if (tmp ==1)
					{
						parts[ID(r)].tmp++;
						parts[i].tmp--;
						break;
					}
					if (tmp>0)
					{
						parts[ID(r)].tmp += tmp/2;
						parts[i].tmp -= tmp/2;
						break;
					}
				}
			}
		}
	}
	return 0;
}

int Element_QRTZ_graphics(GRAPHICS_FUNC_ARGS)
 //QRTZ and PQRT
{
	int z = (cpart->tmp2 - 5) * 16;//speckles!
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = sim->rng.between(0, 10);
	sim->parts[i].tmp3 = int(sim->pv[y/CELL][x/CELL] * 64);
}
