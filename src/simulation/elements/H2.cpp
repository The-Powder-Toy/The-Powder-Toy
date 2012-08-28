#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_H2 PT_H2 148
Element_H2::Element_H2()
{
    Identifier = "DEFAULT_PT_H2";
    Name = "HYGN";
    Colour = PIXPACK(0x5070FF);
    MenuVisible = 1;
    MenuSection = SC_GAS;
    Enabled = 1;
    
    Advection = 2.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.99f;
    Loss = 0.30f;
    Collision = -0.10f;
    Gravity = 0.00f;
    Diffusion = 3.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 1;
    
    Temperature = R_TEMP+0.0f +273.15f;
    HeatConduct = 251;
    Description = "Combines with O2 to make WATR";
    
    State = ST_GAS;
    Properties = TYPE_GAS;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_H2::update;
    
}

//#TPT-Directive ElementHeader Element_H2 static int update(UPDATE_FUNC_ARGS)
int Element_H2::update(UPDATE_FUNC_ARGS)

{
	int r,rx,ry,rt;
	if (parts[i].temp > 2273.15 && sim->pv[y/CELL][x/CELL] > 50.0f)
		parts[i].tmp = 1;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				rt = (r&0xFF);
				if (!r)
					continue;
				if (sim->pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL) // This will not work. DESL turns to fire above 5.0 pressure
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					sim->part_change_type(i,x,y,PT_OIL);
				}
				if (parts[r>>8].temp > 2273.15)// && pv[y/CELL][x/CELL] > 50.0f)
					continue;
				if (parts[i].temp < 2273.15)
				{
					if (rt==PT_FIRE)
					{
						parts[r>>8].temp=2473.15;
						if(parts[r>>8].tmp&0x02)
						parts[r>>8].temp=3473;
						parts[r>>8].tmp |= 1;
					}
					if (rt==PT_FIRE || rt==PT_PLSM || rt==PT_LAVA)
					{
						sim->create_part(i,x,y,PT_FIRE);
						parts[i].temp+=(rand()/(RAND_MAX/100));
						parts[i].tmp |= 1;
					}
				}
			}
	if (parts[i].temp > 2273.15 && sim->pv[y/CELL][x/CELL] > 50.0f)
	{
		if (rand()%5 < 1)
		{
			int j;
			float temp = parts[i].temp;
			sim->create_part(i,x,y,PT_NBLE);

			j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1) parts[j].temp = temp;
			if (!(rand()%10)) { j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC); if (j != -1) parts[j].temp = temp; }
			j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT); if (j != -1) { parts[j].ctype = 0x7C0000; parts[j].temp = temp; }

			j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PLSM); if (j != -1) parts[j].temp = temp;

			parts[i].temp = temp+750+rand()%500;
			sim->pv[y/CELL][x/CELL] += 30;
		}
	}
	return 0;
}


Element_H2::~Element_H2() {}
