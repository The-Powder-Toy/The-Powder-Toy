#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_POLO PT_POLO 182
Element_POLO::Element_POLO()
{
	Identifier = "DEFAULT_PT_POLO";
	Name = "POLO";
	Colour = PIXPACK(0x506030);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 0;

	Weight = 90;

	Temperature = 388.15f; 
	HeatConduct = 251;
	Description = "Polonium, highly radioactive and deadly to STKM";

	Properties = PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 526.95f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_POLO::update;
	Graphics = &Element_POLO::graphics;
}

//#TPT-Directive ElementHeader Element_POLO static int update(UPDATE_FUNC_ARGS)
int Element_POLO::update(UPDATE_FUNC_ARGS)
{
	const int cooldown = 15;
	const int limit = 5;

	int r = sim->photons[y][x];
	if (parts[i].tmp < limit && !parts[i].life)
	{
		if (!(rand()%1000) &&!parts[i].tmp)
		{
			int s = sim->create_part(-3, x, y, PT_NEUT);
			if (s >= 0)
			{
				parts[i].life = cooldown;
				parts[i].tmp ++;

				parts[i].temp = ((parts[i].temp + parts[s].temp) + 600.0f) / 2.0f;
				parts[s].temp = parts[i].temp;
			}
		}

		
		if (!(rand()%10000))
		{
			int s = sim->create_part(-3, x, y, PT_NEUT);
			if (s >= 0)
			{
				parts[i].temp = ((parts[i].temp + parts[r>>8].temp + parts[r>>8].temp) + 600.0f) / 3.0f;
				parts[i].life = cooldown;
				parts[i].tmp ++;

				parts[r>>8].temp = parts[i].temp;

				parts[s].temp = parts[i].temp;
				parts[s].vx = parts[r>>8].vx;
				parts[s].vy = parts[r>>8].vy;
			}
		}
	}
	if (parts[i].tmp2 >= 20)
	{
		sim->part_change_type(i,x,y,PT_PLUT);
		parts[i].temp = (parts[i].temp+600.0f)/2.0f;
		return 1;
	}
	if (parts[r>>8].type == PT_PROT)
	{
		parts[i].tmp2 ++;
		sim->kill_part(r>>8);
	}
	if (parts[i].temp < 388.15f)
	{
		if (parts[i].temp >= 388.15f-0.2f)
		{
			parts[i].temp += 0.2f;
		}
		else
		{
			//no overheating!
			parts[i].temp += 388.15f-parts[i].temp;
		}
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_POLO static int graphics(GRAPHICS_FUNC_ARGS)
int Element_POLO::graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp >= 5)
	{
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	}

	return 0;
}

Element_POLO::~Element_POLO() {}
