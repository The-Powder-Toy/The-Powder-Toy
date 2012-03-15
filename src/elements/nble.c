#include <element.h>

int update_NBLE(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 5273.15 && pv[y/CELL][x/CELL] > 100.0f)
	{
		if (rand()%5 < 1)
		{
			int j;
			float temp = parts[i].temp;
			part_change_type(i,x,y,PT_PLSM);
			parts[i].life = rand()%150+50;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1)
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC); if (j != -1)
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT);
			if (j != -1) { parts[j].ctype = 0xFF0000; parts[j].temp = temp; }

			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_CO2);
			if (j != -1) parts[j].temp = temp - 1000;

			parts[i].temp += 10000;
			pv[y/CELL][x/CELL] += 30;
		}
	}
	return 0;
}
