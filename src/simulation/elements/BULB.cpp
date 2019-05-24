#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_LED PT_LED 197
Element_LED::Element_LED()
{
	Identifier = "DEFAULT_PT_LED";
	Name = "LED";
	Colour = PIXPACK(0x404040);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
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

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 0;
	Description = "Light emitting diode, .tmp2 changes colour modes.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature =NT;
	HighTemperatureTransition = NT;

	Update = &Element_LED::update;
	Graphics = &Element_LED::graphics;
}

//#TPT-Directive ElementHeader Element_LED static int update(UPDATE_FUNC_ARGS)
int Element_LED::update(UPDATE_FUNC_ARGS)

{
	int r, rx, ry, np;
	if (parts[i].life != 10)
	{
		if (parts[i].life > 0)
			parts[i].life--;
	}
	else
	{
		for (rx = -2; rx < 3; rx++)
			for (ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y + ry][x + rx];
					if (!r)
						continue;
					if (TYP(r) == PT_LED)
					{
						if (parts[ID(r)].life < 10 && parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[ID(r)].life == 0)
							parts[ID(r)].life = 10;
					}
				}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_LED static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LED::graphics(GRAPHICS_FUNC_ARGS)
{int gradv;
double tempOver = (((cpart->temp)));
if (cpart->life == 0)
{
	if (cpart->tmp2 == 1)                            // Different tmp modes change colour of glow.
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 250.0);
		*fireg = (int)(gradv * 0.0);
		*fireb = (int)(gradv * 0.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	if (cpart->tmp2 == 2)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 0.0);
		*fireg = (int)(gradv * 250.0);
		*fireb = (int)(gradv * 0.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	if (cpart->tmp2 == 3)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 0.0);
		*fireg = (int)(gradv * 0.0);
		*fireb = (int)(gradv * 250.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	
	if (cpart->tmp2 == 4)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 250.0);
		*fireg = (int)(gradv * 0.0);
		*fireb = (int)(gradv * 250.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	if (cpart->tmp2 == 0)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 250.0);
		*fireg = (int)(gradv * 250.0);
		*fireb = (int)(gradv * 250.0);
		*firea = 50;
		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
}

	return 0;
}
Element_LED::~Element_LED() {}
