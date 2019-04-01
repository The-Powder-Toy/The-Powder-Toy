#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LITH PT_LITH 192
Element_LITH::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0x303030);
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
	HeatConduct = 251;
	Description = "Lithium.(PSCN activates,NSCN deactivates and INST recieve/send charges.)";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = PT_BGLA;

	Update = &Element_LITH::update;
	Graphics = &Element_LITH::graphics;
}

//#TPT-Directive ElementHeader Element_LITH static int update(UPDATE_FUNC_ARGS)
int Element_LITH::update(UPDATE_FUNC_ARGS)

{
	int r, rx, ry, check, setto,np;
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
				if (TYP(r) == PT_LITH && parts[ID(r)].tmp == check)
				{
					parts[ID(r)].tmp = setto;
				}
			}
	for (rx = -3; rx < 3; rx++)
		for (ry = -3; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_INST:
					if (parts[i].temp > 274.15f && parts[i].life == 0)
					{
						parts[i].temp -= 1;
						np = sim->create_part(ID(r), x + rx, y + ry, PT_SPRK);

					}

				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_LITH static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LITH::graphics(GRAPHICS_FUNC_ARGS)
{
	return 0;
}


Element_LITH::~Element_LITH() {}
