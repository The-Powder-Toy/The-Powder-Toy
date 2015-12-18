#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PSNS PT_PSNS 172
Element_PSNS::Element_PSNS()
{
	Identifier = "DEFAULT_PT_PSNS";
	Name = "PSNS";
	Colour = PIXPACK(0xDB2020);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
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

	Temperature = 277.15f;
	HeatConduct = 0;
	Description = "Pressure sensor, creates a spark when the pressure is greater than its temperature.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PSNS::update;
}

//#TPT-Directive ElementHeader Element_PSNS static int update(UPDATE_FUNC_ARGS)
int Element_PSNS::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	if (sim->pv[y/CELL][x/CELL] > parts[i].temp-273.15f)
	{
		parts[i].life = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = r&0xFF;
					if (sim->parts_avg(i,r>>8,PT_INSL) != PT_INSL)
					{
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0)
						{
							parts[r>>8].life = 4;
							parts[r>>8].ctype = rt;
							sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						}
					}
				}
	}
	return 0;
}



Element_PSNS::~Element_PSNS() {}
