#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_DLAY()
{
	Identifier = "DEFAULT_PT_DLAY";
	Name = "DLAY";
	Colour = 0x753590_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWERED;
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

	DefaultProperties.temp = 4.0f + 273.15f;
	HeatConduct = 0;
	Description = "Conducts with temperature-dependent delay. (use HEAT/COOL).";

	Properties = TYPE_SOLID;

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
	auto oldl = parts[i].life;
	if (parts[i].life>0)
		parts[i].life--;
	if (parts[i].temp<= 1.0f+273.15f)
		parts[i].temp = 1.0f+273.15f;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				auto pavg = sim->parts_avg(ID(r), i, PT_INSL);
				if (!r || pavg==PT_INSL || pavg==PT_RSSS)
					continue;
				if (TYP(r)==PT_SPRK && parts[i].life==0 && parts[ID(r)].life>0 && parts[ID(r)].life<4 && parts[ID(r)].ctype==PT_PSCN)
				{
					parts[i].life = (int)(parts[i].temp-273.15f+0.5f);
				}
				else if (TYP(r)==PT_DLAY)
				{
					if (!parts[i].life)
					{
						if (parts[ID(r)].life)
						{
							parts[i].life = parts[ID(r)].life;
							if((ID(r))>i) //If the other particle hasn't been life updated
								parts[i].life--;
						}
					}
					else if (!parts[ID(r)].life)
					{
						parts[ID(r)].life = parts[i].life;
						if((ID(r))>i) //If the other particle hasn't been life updated
							parts[ID(r)].life++;
					}
				}
				else if(TYP(r)==PT_NSCN && oldl==1)
				{
					sim->create_part(-1, x+rx, y+ry, PT_SPRK);
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int stage = (int)(((float)cpart->life/(cpart->temp-273.15))*100.0f);
	*colr += stage;
	*colg += stage;
	*colb += stage;
	return 0;
}
