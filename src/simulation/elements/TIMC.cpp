#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_TIMC()
{
	Identifier = "DEFAULT_PT_TIMC";
	Name = "TIMC";
	Colour = PIXPACK(0x336699);
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
	Hardness = 0;

	Weight = 100;
	HeatConduct = 0;
	Description = "Time crystal, converts into it's ctype when sparked with PSCN, Configure delay with .tmp";

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
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	{
		int r, rx, ry, oldl;
		oldl = parts[i].tmp2;
		if (parts[i].tmp2 > 0)
			parts[i].tmp2--;
		for (rx = -2; rx < 3; rx++)
			for (ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y + ry][x + rx];
					if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
						continue;
					if (TYP(r) == PT_SPRK && parts[i].life == 0 && parts[ID(r)].life > 0 && parts[ID(r)].life < 4 && parts[ID(r)].ctype == PT_PSCN)
					{
						parts[i].tmp2 = (int)(parts[i].tmp);
					}
					else if (TYP(r) == PT_TIMC)
					{
						if (!parts[i].tmp2)
						{
							if (parts[ID(r)].tmp2)
							{
								parts[i].tmp2 = parts[ID(r)].tmp2;
								if ((ID(r)) > i)
									parts[i].tmp2--;
							}
						}
						else if (!parts[ID(r)].tmp2)
						{
							parts[ID(r)].tmp2 = parts[i].tmp2;
							if ((ID(r)) > i)
								parts[ID(r)].tmp2++;
						}
					}
					if (parts[i].tmp2 == 1)
					{
						parts[i].type = parts[i].ctype;
					}
				}
		//}
		return 0;
	}
}

static int graphics(GRAPHICS_FUNC_ARGS)
{

	int stage = (int)(((float)cpart->tmp2 / (cpart->temp - 273.15))*100.0f);
	*colr += stage;
	*colg += stage;
	*colb += stage;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp = 100;
}
