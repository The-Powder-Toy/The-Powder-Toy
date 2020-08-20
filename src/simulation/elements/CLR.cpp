#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
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

	HeatConduct = 0;
	Description = "Electronic eraser, clears surrounding when sparked with PSCN & NSCN. Use .tmp to set radius. Read wiki!";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	DefaultProperties.tmp = 10;
	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int range = parts[i].tmp;
	if (parts [i].tmp > 100|| parts[i].tmp < 0)
		parts[i].tmp = 10;
	if (parts[i].life > 0)
		parts[i].life--;

	for (int rx = -range; rx < range + 1; rx++)
		for (int ry = -range; ry < range + 1; ry++)
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				if (parts[i].life > 0 && parts[i].life < 14 && parts[ID(r)].type != PT_PSCN && parts[ID(r)].type != PT_WIFI && parts[ID(r)].ctype != PT_PSCN)
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_NONE);
					continue;
				}
				else if  (parts[i].life >= 14 )
				{
					sim->part_change_type(ID(r), x + rx, y + ry, PT_NONE);
					sim->part_change_type(i, x, y, PT_NONE);
					continue;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life > 0 )
	{
		*colr = 255;
		*colg = 0;
		*colb = 0;
		*pixel_mode |= PMODE_LFLARE;
	}

	if (cpart->life > 15)
	{	*colr = 0;
		*colg = 0;
		*colb = 255;
		*pixel_mode |= PMODE_LFLARE;
	}
		return 0;
}
