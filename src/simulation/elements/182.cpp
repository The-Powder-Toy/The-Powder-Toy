#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_E182 PT_E182 182

/*
TODO: 
	- Molten version
	- "muted version" a powder created by mixing E182 with ? that is weaker
*/

Element_E182::Element_E182()
{
	Identifier = "DEFAULT_PT_E182";
	Name = "E182";
	Colour = PIXPACK(0x506030);
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
	Description = "Experimental element. Some kind of nuclear fuel";

	Properties = PROP_NEUTPASS|PROP_RADIOACTIVE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_E182::update;
	Graphics = &Element_E182::graphics;
}

//#TPT-Directive ElementHeader Element_E182 static int update(UPDATE_FUNC_ARGS)
int Element_E182::update(UPDATE_FUNC_ARGS)
{
	const int cooldown = 15;
	const int limit = 5;
	int r, s;
	if(parts[i].tmp < limit && !parts[i].life)
	{
		if (!(rand()%10000) && !parts[i].tmp)
		{
			s = sim->create_part(-3, x, y, PT_NEUT);
			if(s >= 0) {
				parts[i].life = cooldown;
				parts[i].tmp ++;

				parts[i].temp = ((parts[i].temp + parts[s].temp) + 600.0f) / 2.0f;
				parts[s].temp = parts[i].temp;
			}
		}

		r = sim->photons[y][x];
		if (r && !(rand()%100)) {
			s = sim->create_part(-3, x, y, PT_NEUT);
			if(s >= 0) {
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
	return 0;
}


//#TPT-Directive ElementHeader Element_E182 static int graphics(GRAPHICS_FUNC_ARGS)
int Element_E182::graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp >= 5) {
		*colr = 0x70;
		*colg = 0x70;
		*colb = 0x70;
	}

	return 0;
}

Element_E182::~Element_E182() {}
