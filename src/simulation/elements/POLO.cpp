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
	PhotonReflectWavelengths = 0x000FF200;

	Weight = 90;

	Temperature = 388.15f; 
	HeatConduct = 251;
	Description = "Polonium, highly radioactive. Decays into NEUT and heats up.";

	Properties = TYPE_PART|PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC|PROP_DEADLY;

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

#define COOLDOWN 15
#define LIMIT 5

//#TPT-Directive ElementHeader Element_POLO static int update(UPDATE_FUNC_ARGS)
int Element_POLO::update(UPDATE_FUNC_ARGS)
{
	int r = sim->photons[y][x];
	if (parts[i].tmp < LIMIT && !parts[i].life)
	{
		if (!(rand()%10000) && !parts[i].tmp)
		{
			int s = sim->create_part(-3, x, y, PT_NEUT);
			if (s >= 0)
			{
				parts[i].life = COOLDOWN;
				parts[i].tmp++;

				parts[i].temp = ((parts[i].temp + parts[s].temp) + 600.0f) / 2.0f;
				parts[s].temp = parts[i].temp;
			}
		}

		if (r && !(rand()%100))
		{
			int s = sim->create_part(-3, x, y, PT_NEUT);
			if (s >= 0)
			{
				parts[i].temp = ((parts[i].temp + parts[ID(r)].temp + parts[ID(r)].temp) + 600.0f) / 3.0f;
				parts[i].life = COOLDOWN;
				parts[i].tmp++;

				parts[ID(r)].temp = parts[i].temp;

				parts[s].temp = parts[i].temp;
				parts[s].vx = parts[ID(r)].vx;
				parts[s].vy = parts[ID(r)].vy;
			}
		}
	}
	if (parts[i].tmp2 >= 10)
	{
		sim->part_change_type(i,x,y,PT_PLUT);
		parts[i].temp = (parts[i].temp+600.0f)/2.0f;
		return 1;
	}
	if (parts[ID(r)].type == PT_PROT)
	{
		parts[i].tmp2++;
		sim->kill_part(ID(r));
	}
	if (parts[i].temp < 388.15f)
	{
		parts[i].temp += 0.2f;
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_POLO static int graphics(GRAPHICS_FUNC_ARGS)
int Element_POLO::graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp >= LIMIT)
	{
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	}
	else
		*pixel_mode |= PMODE_GLOW;

	return 0;
}

Element_POLO::~Element_POLO() {}
