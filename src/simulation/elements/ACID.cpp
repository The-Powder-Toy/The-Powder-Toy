#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_ACID()
{
	Identifier = "DEFAULT_PT_ACID";
	Name = "ACID";
	Colour = 0xED55FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 40;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x1FE001FE;

	Weight = 10;

	HeatConduct = 34;
	Description = "Dissolves almost everything.";

	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 75;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				int rt = TYP(r);
				if (rt != PT_ACID && rt != PT_CAUS)
				{
					if (rt == PT_PLEX || rt == PT_NITR || rt == PT_GUNP || rt == PT_RBDM || rt == PT_LRBD)
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_FIRE);
						parts[i].life = 4;
						parts[ID(r)].life = 4;
					}
					else if (rt == PT_WTRV)
					{
						if (sim->rng.chance(1, 250))
						{
							sim->part_change_type(i, x, y, PT_CAUS);
							parts[i].life = sim->rng.between(25, 74);
							sim->kill_part(ID(r));
						}
					}
					else if (rt != PT_CLNE && rt != PT_PCLN && parts[i].life > 50 && sim->rng.chance(elements[rt].Hardness, 1000))
					{
						if (sim->parts_avg(i, ID(r),PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
							float newtemp = ((60.0f-(float)elements[rt].Hardness))*7.0f;
							if(newtemp < 0){
								newtemp = 0;
							}
							parts[i].temp += newtemp;
							parts[i].life--;
							switch (rt)
							{
							case PT_LITH:
								sim->part_change_type(ID(r), x + rx, y + ry, PT_H2);
								break;

							default:
								sim->kill_part(ID(r));
								break;
							}
						}
					}
					else if (parts[i].life<=50)
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
		}
	}
	for (auto trade = 0; trade<2; trade++)
	{
		auto rx = sim->rng.between(-2, 2);
		auto ry = sim->rng.between(-2, 2);
		if (rx || ry)
		{
			auto r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r) == PT_ACID && (parts[i].life > parts[ID(r)].life) && parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[ID(r)].life;
				if (temp == 1)
				{
					parts[ID(r)].life++;
					parts[i].life--;
				}
				else if (temp>0)
				{
					parts[ID(r)].life += temp/2;
					parts[i].life -= temp/2;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int s = cpart->life;
	if (s>75) s = 75; //These two should not be here.
	if (s<49) s = 49;
	s = (s-49)*3;
	if (s==0) s = 1;
	*colr += s*4;
	*colg += s*1;
	*colb += s*2;
	*pixel_mode |= PMODE_BLUR;
	return 0;
}
