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
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Caustic Gas, acts like ACID.";
	
	State = ST_GAS;
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
	int r, rx, ry, trade, np;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_ACID && (r&0xFF)!=PT_CAUS)
				{
					if (((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && sim->elements[r&0xFF].Hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (sim->parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
							float newtemp = ((60.0f-(float)sim->elements[r&0xFF].Hardness))*7.0f;
							if(newtemp < 0){
								newtemp = 0;
							}
							parts[i].temp += newtemp;
							parts[i].life--;
							sim->kill_part(r>>8);
						}
					}
					else if (parts[i].life<=50)
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}


Element_CAUS::~Element_CAUS() {}
