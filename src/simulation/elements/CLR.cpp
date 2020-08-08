#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
void Element::Element_ECLR()
{
	Identifier = "DEFAULT_PT_ECLR";
	Name = "ECLR";
	Colour = PIXPACK(0xD6D1D4);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 35;
	Description = "Electronic eraser, clears surrounding when sparked with PSCN. Use. tmp to set radius.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2887.15f;
	HighTemperatureTransition = ST;
	DefaultProperties.tmp = 10;
	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int range = parts[i].tmp;
	if (parts [i].tmp > 100|| parts[i].tmp < 0)
		parts[i].tmp = 10;
	for (int rx = -range; rx < range + 1; rx++)
		for (int ry = -range; ry < range + 1; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				if (parts[i].life == 1 && parts[ID(r)].type != PT_PSCN && parts[ID(r)].type != PT_SPRK)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_NONE);
					continue;
				}
				if (parts[ID(r)].type == PT_SPRK)
				{
						parts[i].life = 1;
				}
				else
				{
					parts[i].life = 0;
				}
			}
	return 0;
}
