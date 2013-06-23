#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PRPC PT_PRPC 175
Element_PRPC::Element_PRPC()
{
	Identifier = "DEFAULT_PT_PRPC";
	Name = "PRPC";
	Colour = PIXPACK(0xFF6600);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
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
	Hardness = 0;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Solid. Duplicates any properties to any particle it touches.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_PRPC::update;
	
}

int Element_PRPC::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if((r&0xFF)!=PT_PRPC)
					{
						if(parts[i].ctype!=PT_NONE)parts[r>>8].ctype = parts[i].ctype;
						parts[r>>8].tmp = parts[i].tmp;
						if(parts[i].tmp2!=0)parts[r>>8].tmp2 = parts[i].tmp2;
						parts[r>>8].life = parts[i].life;
						parts[r>>8].temp = parts[i].temp;
					}
				}
	return(0);
}
Element_PRPC::~Element_PRPC() {}