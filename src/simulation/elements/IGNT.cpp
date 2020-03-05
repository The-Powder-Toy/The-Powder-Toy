#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_IGNT()
{
	Identifier = "DEFAULT_PT_IGNT";
	Name = "IGNC";
	Colour = PIXPACK(0xC0B050);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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
	Hardness = 1;

	Weight = 100;

	HeatConduct = 88;
	Description = "Ignition cord. Burns slowly with fire and sparks.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE | PROP_SPARKSETTLE | PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_FIRE;

	DefaultProperties.life = 3;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	 int r, rx, ry, rt;
	if(parts[i].tmp==0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (rt==PT_FIRE || rt==PT_PLSM || rt==PT_SPRK || rt==PT_LIGH || (rt==PT_IGNT && parts[ID(r)].life==1))
					{
						parts[i].tmp = 1;
					}
				}
	}
	else if(parts[i].life > 0)
	{
		if (RNG::Ref().chance(2, 3))
		{
			int nb = sim->create_part(-1, x + RNG::Ref().between(-1, 1), y + RNG::Ref().between(-1, 1), PT_EMBR);
			if (nb!=-1) {
				parts[nb].tmp = 0;
				parts[nb].life = 30;
				parts[nb].vx = RNG::Ref().between(-10, 10);
				parts[nb].vy = RNG::Ref().between(-10, 10);
				parts[nb].temp = restrict_flt(parts[i].temp-273.15f+400.0f, MIN_TEMP, MAX_TEMP);
			}
		}
		else
		{
			sim->create_part(-1, x + RNG::Ref().between(-1, 1), y + RNG::Ref().between(-1, 1), PT_FIRE);
		}
		parts[i].life--;
	}
	return 0;
}
