#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_RPEL()
{
	Identifier = "DEFAULT_PT_RPEL";
	Name = "RPEL";
	Colour = PIXPACK(0x99CC00);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = 20.0f + 273.15f;
	HeatConduct = 0;
	Description = "Repels or attracts particles based on its temperature.";

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
	CtypeDraw = &Element::basicCtypeDraw;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, ri;
	for(ri = 0; ri <= 10; ri++)
	{
		rx = RNG::Ref().between(-10, 10);
		ry = RNG::Ref().between(-10, 10);
		if (x+rx >= 0 && x+rx < XRES && y+ry >= 0 && y+ry < YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				r = sim->photons[y+ry][x+rx];

			if (r && !(sim->elements[TYP(r)].Properties & TYPE_SOLID)) {
				if (!parts[i].ctype || parts[i].ctype == parts[ID(r)].type) {
					parts[ID(r)].vx += isign(rx)*((parts[i].temp-273.15)/10.0f);
					parts[ID(r)].vy += isign(ry)*((parts[i].temp-273.15)/10.0f);
				}
			}
		}
	}
	return 0;
}
