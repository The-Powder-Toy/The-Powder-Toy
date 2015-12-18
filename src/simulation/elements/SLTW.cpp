#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SLTW PT_SLTW 27
Element_SLTW::Element_SLTW()
{
	Identifier = "DEFAULT_PT_SLTW";
	Name = "SLTW";
	Colour = PIXPACK(0x4050F0);
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

	Weight = 35;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 75;
	Description = "Saltwater, conducts electricity, difficult to freeze.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 252.05f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 383.0f;
	HighTemperatureTransition = ST;

	Update = &Element_SLTW::update;
}

//#TPT-Directive ElementHeader Element_SLTW static int update(UPDATE_FUNC_ARGS)
int Element_SLTW::update(UPDATE_FUNC_ARGS)
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
					if (!(rand()%2000))
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
					break;
				case PT_PLNT:
					if (!(rand()%40))
						sim->kill_part(r>>8);
					break;
				case PT_RBDM:
				case PT_LRBD:
					if ((sim->legacy_enable||parts[i].temp>(273.15f+12.0f)) && !(rand()%100))
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						parts[i].life = 4;
						parts[i].ctype = PT_WATR;
					}
					break;
				case PT_FIRE:
					if (parts[r>>8].ctype!=PT_WATR)
					{
						sim->kill_part(r>>8);
						if(!(rand()%30)){
							sim->kill_part(i);
							return 1;
						}
					}
					break;
				case PT_NONE:
					break;
				default:
					continue;
				}
			}
	return 0;
}


Element_SLTW::~Element_SLTW() {}
