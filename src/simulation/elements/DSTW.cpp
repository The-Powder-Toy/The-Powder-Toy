#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DSTW PT_DSTW 25
Element_DSTW::Element_DSTW()
{
	Identifier = "DEFAULT_PT_DSTW";
	Name = "DSTW";
	Colour = PIXPACK(0x1020C0);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	Temperature = R_TEMP-2.0f	+273.15f;
	HeatConduct = 23;
	Description = "Distilled water, does not conduct electricity.";

	Properties = TYPE_LIQUID|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 273.15f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 373.0f;
	HighTemperatureTransition = PT_WTRV;

	Update = &Element_DSTW::update;
}

//#TPT-Directive ElementHeader Element_DSTW static int update(UPDATE_FUNC_ARGS)
int Element_DSTW::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				switch (r&0xFF)
				{
				case PT_SALT:
					if (!(rand()%50))
					{
						sim->part_change_type(i,x,y,PT_SLTW);
						// on average, convert 3 DSTW to SLTW before SALT turns into SLTW
						if (!(rand()%3))
							sim->part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
					}
					break;
				case PT_SLTW:
					if (!(rand()%2000))
					{
						sim->part_change_type(i,x,y,PT_SLTW);
						break;
					}
				case PT_WATR:
					if (!(rand()%100))
					{
						sim->part_change_type(i,x,y,PT_WATR);
					}
					break;
				case PT_RBDM:
				case PT_LRBD:
					if ((sim->legacy_enable||parts[i].temp>12.0f) && !(rand()%100))
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						parts[i].life = 4;
					}
					break;
				case PT_FIRE:
					sim->kill_part(r>>8);
					if(!(rand()%30)){
						sim->kill_part(i);
						return 1;
					}
					break;
				default:
					continue;
				}
			}
	return 0;
}


Element_DSTW::~Element_DSTW() {}
