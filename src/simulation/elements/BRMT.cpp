#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BRMT()
{
	Identifier = "DEFAULT_PT_BRMT";
	Name = "BRMT";
	Colour = PIXPACK(0x705060);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 90;

	HeatConduct = 211;
	Description = "Broken metal. Created when iron rusts or when metals break from pressure.";

	Properties = TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = ST;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, tempFactor;
	if (parts[i].temp > 523.15f)//250.0f+273.15f
	{
		tempFactor = int(1000 - ((523.15f-parts[i].temp)*2));
		if(tempFactor < 2)
			tempFactor = 2;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_BREC && RNG::Ref().chance(1, tempFactor))
					{
						if (RNG::Ref().chance(1, 2))
						{
							sim->create_part(ID(r), x+rx, y+ry, PT_THRM);
						}
						else
							sim->create_part(i, x, y, PT_THRM);
					}
				}
	}
	return 0;
}
