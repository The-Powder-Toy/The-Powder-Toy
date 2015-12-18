#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PUMP PT_PUMP 97
Element_PUMP::Element_PUMP()
{
	Identifier = "DEFAULT_PT_PUMP";
	Name = "PUMP";
	Colour = PIXPACK(0x0A0A3B);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	Temperature = 273.15f;
	HeatConduct = 0;
	Description = "Pressure pump. Changes pressure to its temp when activated. (use HEAT/COOL).";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PUMP::update;
	Graphics = &Element_PUMP::graphics;
}

//#TPT-Directive ElementHeader Element_PUMP static int update(UPDATE_FUNC_ARGS)
int Element_PUMP::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		if (parts[i].temp>=256.0+273.15)
			parts[i].temp=256.0+273.15;
		if (parts[i].temp<= -256.0+273.15)
			parts[i].temp = -256.0+273.15;

		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (!(rx && ry))
				{
					sim->pv[(y/CELL)+ry][(x/CELL)+rx] += 0.1f*((parts[i].temp-273.15)-sim->pv[(y/CELL)+ry][(x/CELL)+rx]);
				}
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_PUMP)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_PUMP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PUMP::graphics(GRAPHICS_FUNC_ARGS)

{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colb += lifemod;
	return 0;
}


Element_PUMP::~Element_PUMP() {}
