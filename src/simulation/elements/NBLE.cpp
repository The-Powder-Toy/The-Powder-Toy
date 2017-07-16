#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_NBLE PT_NBLE 52
Element_NBLE::Element_NBLE()
{
	Identifier = "DEFAULT_PT_NBLE";
	Name = "NBLE";
	Colour = PIXPACK(0xEB4917);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.75f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	PhotonReflectWavelengths = 0x3FFF8000;

	Weight = 1;

	Temperature = R_TEMP+2.0f	+273.15f;
	HeatConduct = 106;
	Description = "Noble Gas. Diffuses and conductive. Ionizes into plasma when introduced to electricity.";

	Properties = TYPE_GAS|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_NBLE::update;
}

//#TPT-Directive ElementHeader Element_NBLE static int update(UPDATE_FUNC_ARGS)
int Element_NBLE::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 5273.15 && sim->pv[y/CELL][x/CELL] > 100.0f)
	{
		parts[i].tmp |= 0x1;
		if (!(rand()%5))
		{
			int j;
			float temp = parts[i].temp;
			sim->create_part(i,x,y,PT_CO2);

			j = sim->create_part(-3,x,y,PT_NEUT);
			if (j != -1)
				parts[j].temp = temp;
			if (!(rand()%25))
			{
				j = sim->create_part(-3,x,y,PT_ELEC);
				if (j != -1)
					parts[j].temp = temp;
			}
			j = sim->create_part(-3,x,y,PT_PHOT);
			if (j != -1)
			{
				parts[j].ctype = 0xF800000;
				parts[j].temp = temp;
				parts[j].tmp = 0x1;
			}
			int rx = x+rand()%3-1, ry = y+rand()%3-1, rt = pmap[ry][rx]&0xFF;
			if (sim->can_move[PT_PLSM][rt] || rt == PT_NBLE)
			{
				j = sim->create_part(-3,rx,ry,PT_PLSM);
				if (j != -1)
				{
					parts[j].temp = temp;
					parts[j].tmp |= 4;
				}
			}
			parts[i].temp = temp+1750+rand()%500;
			sim->pv[y/CELL][x/CELL] += 50;
		}
	}
	return 0;
}


Element_NBLE::~Element_NBLE() {}
