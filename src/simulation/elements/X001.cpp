#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_X001 PT_X001 185

/*
TODO: 
	- Molten version
	- "muted version" a powder created by mixing X001 with ? that is weaker
*/

Element_X001::Element_X001()
{
	Identifier = "DEFAULT_PT_X001";
	Name = "X001";
	Colour = PIXPACK(0x80C030);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
#if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
#else
	Enabled = 0;
#endif

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
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	Temperature = R_TEMP+4.0f	+273.15f;
	HeatConduct = 251;
	Description = "Experimental element.";

	Properties = PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_X001::update;
	Graphics = &Element_X001::graphics;
}

//#TPT-Directive ElementHeader Element_X001 static int update(UPDATE_FUNC_ARGS)
int Element_X001::update(UPDATE_FUNC_ARGS)
{
	int r, s, rx, ry, rr;
	const int cooldown = 15;
	const int limit = 10;
	rr = sim->photons[y][x];
	if(parts[i].tmp < limit && !parts[i].life)
	{
		if (!(rand()%8000) && !parts[i].tmp)
		{
			s = sim->create_part(-3, x, y, PT_ELEC);
			if (s >= 0)
			{
				parts[i].life = cooldown;
				parts[i].tmp = 1;
				parts[i].temp += 10;
				parts[s].temp = parts[i].temp;
			}
		}
		if (rr && (rr & 0xFF) != PT_NEUT && !(rand()%80))
		{
			if (rand() % 3)
				s = sim->create_part(-3, x, y, PT_ELEC);
			else
				s = sim->create_part(-3, x, y, PT_X002);
			parts[i].life = cooldown;
			parts[i].tmp ++;
			parts[i].temp += 10;
			parts[rr>>8].temp = parts[i].temp;
			parts[s].temp = parts[i].temp;
		}
	}
	if ((rr & 0xFF) == PT_NEUT && !(rand()%10))
	{
		s = parts[i].tmp;
		if (s) parts[i].tmp --;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r & 0xFF) == PT_E182 && !(rand()%40))
				{
					if (rand()%4)
						parts[i].tmp = 0;
					parts[r>>8].tmp = 0;
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_X001 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_X001::graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp >= 10) {
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	}
	return 0;
}

Element_X001::~Element_X001() {}
