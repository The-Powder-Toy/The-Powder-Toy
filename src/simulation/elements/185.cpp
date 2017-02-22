#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E185 PT_E185 185

/*
TODO: 
	- Molten version
	- "muted version" a powder created by mixing E185 with ? that is weaker
*/

Element_E185::Element_E185()
{
	Identifier = "DEFAULT_PT_E185";
	Name = "E185";
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
	Description = "Experimental element. Some kind of nuclear fuel.";

	Properties = TYPE_PART|PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E185::update;
	Graphics = &Element_E185::graphics;
}

//#TPT-Directive ElementHeader Element_E185 static int update(UPDATE_FUNC_ARGS)
int Element_E185::update(UPDATE_FUNC_ARGS)
{
	int r, s, rx, ry, rr, sctype, stmp;
	const int cooldown = 15;
	const int limit = 20;
	rr = sim->photons[y][x];
	stmp = parts[i].tmp;
	if(stmp < limit && !parts[i].life)
	{
		sctype = parts[i].ctype;
		if (!(rand()%140) && !(rand()%100) && !stmp)
		{
			if (!sctype)
				s = sim->create_part(-3, x, y, PT_ELEC);
			else
				s = sim->create_part(-3, x, y, sctype);
			if (s >= 0)
			{
				parts[i].life = cooldown;
				parts[i].tmp = 1;
				parts[i].temp += 10;
				parts[s].temp = parts[i].temp;
			}
		}
		if (rr && (rr & 0xFF) != PT_NEUT && !(rand()%80) && ((stmp - 11) < rand() % 10))
		{
			if (rand() % 10)
			{
				if (!sctype)
					s = sim->create_part(-3, x, y, PT_ELEC);
				else
					s = sim->create_part(-3, x, y, sctype);
			}
			else
				s = sim->create_part(-3, x, y, PT_E186);
			parts[i].life = cooldown;
			parts[i].tmp ++;
			parts[i].temp += (stmp >= 10) ? (stmp - 8) * 10 : 10;

			parts[rr>>8].temp = parts[i].temp;
			if (s >= 0)
			{
				parts[s].ctype = sctype;
				parts[s].temp = parts[i].temp;
			}
		}
	}
	if ((rr & 0xFF) == PT_NEUT && !(rand()%10))
	{
		s = parts[i].tmp;
		parts[i].tmp -= s > 0 ? (s >> 3) + 1 : 0;
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


//#TPT-Directive ElementHeader Element_E185 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E185::graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp >= 20) {
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	} else if (cpart->tmp >= 10) {
		*colr = 0x78;
		*colg = 0x98;
		*colb = 0x50;
	}
	return 0;
}

Element_E185::~Element_E185() {}
