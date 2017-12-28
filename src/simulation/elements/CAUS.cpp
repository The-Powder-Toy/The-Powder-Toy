#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CAUS PT_CAUS 86
Element_CAUS::Element_CAUS()
{
	Identifier = "DEFAULT_PT_CAUS";
	Name = "CAUS";
	Colour = PIXPACK(0x80FFA0);
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 1.50f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	Temperature = R_TEMP+273.15f;
	HeatConduct = 70;
	Description = "Caustic Gas, acts like ACID.";

	Properties = TYPE_GAS|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_CAUS::update;
}

//#TPT-Directive ElementHeader Element_CAUS static int update(UPDATE_FUNC_ARGS)
int Element_CAUS::update(UPDATE_FUNC_ARGS)
{
	for (int rx = -2; rx <= 2; rx++)
		for (int ry = -2; ry <= 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r) == PT_GAS)
				{
					if (sim->pv[(y+ry)/CELL][(x+rx)/CELL] > 3)
					{
						sim->part_change_type(ID(r), x+rx, y+ry, PT_RFRG);
						sim->part_change_type(i, x, y, PT_RFRG);
					}
				}
				else if (TYP(r) != PT_ACID && TYP(r) != PT_CAUS && TYP(r) != PT_RFRG && TYP(r) != PT_RFGL)
				{
					if ((TYP(r) != PT_CLNE && TYP(r) != PT_PCLN && sim->elements[TYP(r)].Hardness > (rand()%1000)) && parts[i].life >= 50)
					{
						// GLAS protects stuff from acid
						if (sim->parts_avg(i, ID(r),PT_GLAS) != PT_GLAS)
						{
							float newtemp = ((60.0f - (float)sim->elements[TYP(r)].Hardness)) * 7.0f;
							if (newtemp < 0)
								newtemp = 0;
							parts[i].temp += newtemp;
							parts[i].life--;
							sim->kill_part(ID(r));
						}
					}
					else if (parts[i].life <= 50)
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}


Element_CAUS::~Element_CAUS() {}
