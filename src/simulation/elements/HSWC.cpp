#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_HSWC PT_HSWC 75
Element_HSWC::Element_HSWC()
{
	Identifier = "DEFAULT_PT_HSWC";
	Name = "HSWC";
	Colour = PIXPACK(0x3B0A0A);
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Heat switch. Conducts heat only when activated.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_HSWC::update;
	Graphics = &Element_HSWC::graphics;
}

//#TPT-Directive ElementHeader Element_HSWC static int update(UPDATE_FUNC_ARGS)
int Element_HSWC::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_HSWC)
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


//#TPT-Directive ElementHeader Element_HSWC static int graphics(GRAPHICS_FUNC_ARGS)
int Element_HSWC::graphics(GRAPHICS_FUNC_ARGS)

{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colr += lifemod;
	return 0;
}


Element_HSWC::~Element_HSWC() {}
