#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LED PT_LED 197
Element_LED::Element_LED()
{
	Identifier = "DEFAULT_PT_LED";
	Name = "LED";
	Colour = PIXPACK(0x505050);
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
	Description = "LED,Glows when activated)";

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
	int r, rx, ry, check, setto;
	switch (parts[i].tmp)
	{
	case 1:
		if (parts[i].life <= 0)
			parts[i].tmp = 0;
		else
		{
			parts[i].life -= 2;
			if (parts[i].life < 0)
				parts[i].life = 0;
			parts[i].tmp2 = parts[i].life;
		}
	case 0:
		check = 3;
		setto = 1;
		break;
	case 2:
		if (parts[i].life >= 10)
			parts[i].tmp = 3;
		else
		{
			parts[i].life += 2;
			if (parts[i].life > 10)
				parts[i].life = 10;
			parts[i].tmp2 = parts[i].life;
		}
	case 3:
		check = 0;
		setto = 2;
		break;
	default:
		parts[i].tmp = 0;
		parts[i].life = 0;
		return 0;
	}
	for (rx = -1; rx < 2; rx++)
		for (ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				if (TYP(r) == PT_LED && parts[ID(r)].tmp == check)
				{
					parts[ID(r)].tmp = setto;
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_LED static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LED::graphics(GRAPHICS_FUNC_ARGS)
{
	int gradv;
	double tempOver = (((cpart->life)));
	if (cpart->life > 5)
	{
		double gradv = sin(tempOver) + 20.0;
		*firer = (int)(gradv * 158.0);
		*fireg = (int)(gradv * 156.0);
		*fireb = (int)(gradv * 112.0);
		*firea = 50;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}
Element_LED::~Element_LED() {}
