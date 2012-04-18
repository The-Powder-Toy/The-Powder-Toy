#include "simulation/Element.h"

int update_NBLE(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 5273.15 && sim->pv[y/CELL][x/CELL] > 50.0f)
	{
		parts[i].tmp = 1;
		if (rand()%5 < 1)
		{
			int j;
			float temp = parts[i].temp;
			sim->part_change_type(i,x,y,PT_PLSM);
			parts[i].life = rand()%150+50;
			sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT);
			sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC);
			j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT);
			if (j != -1) { parts[j].ctype = 0xFF0000; parts[j].temp = temp; }

			j = sim->create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_CO2);
			if (j != -1) parts[j].temp = temp;

			parts[i].temp += 10000;
			sim->pv[y/CELL][x/CELL] += 30;
		}
	}
	return 0;
}
