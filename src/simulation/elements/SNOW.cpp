#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SNOW()
{
	Identifier = "DEFAULT_PT_SNOW";
	Name = "SNOW";
	Colour = 0xC0E0FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.90f;
	Collision = -0.1f;
	Gravity = 0.05f;
	Diffusion = 0.01f;
	HotAir = -0.00005f* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	PhotonReflectWavelengths = 0x03FFFFFF;

	Weight = 50;

	DefaultProperties.temp = R_TEMP - 30.0f + 273.15f;
	HeatConduct = 46;
	LatentHeat = 1095;
	Description = "Light particles. Created when ICE breaks under pressure.";

	Properties = TYPE_PART|PROP_NEUTPASS;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 252.05f;
	HighTemperatureTransition = ST;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].ctype==PT_FRZW)//get colder if it is from FRZW
	{
		parts[i].temp = restrict_flt(parts[i].temp-1.0f, MIN_TEMP, MAX_TEMP);
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
				if ((TYP(r)==PT_SALT || TYP(r)==PT_SLTW) && sim->rng.chance(1, 333))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SLTW);
				}
			}
		}
	}
	return 0;
}
