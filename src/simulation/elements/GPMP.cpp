#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GPMP()
{
	Identifier = "DEFAULT_PT_GPMP";
	Name = "GPMP";
	Colour = PIXPACK(0x0A3B3B);
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
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "Gravity pump. Changes gravity to its temp when activated. (use HEAT/COOL)";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 10;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		if (parts[i].temp>=256.0f+273.15f)
			parts[i].temp=256.0f+273.15f;
		if (parts[i].temp<= -256.0f+273.15f)
			parts[i].temp = -256.0f+273.15f;

		sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = 0.2f*(parts[i].temp-273.15);
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_GPMP)
					{
						if (parts[ID(r)].life<10&&parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[ID(r)].life==0)
							parts[ID(r)].life = 10;
					}
				}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colg += lifemod;
	*colb += lifemod;
	return 0;
}
