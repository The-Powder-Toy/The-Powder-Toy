/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <element.h>

int update_H2(UPDATE_FUNC_ARGS)
{
	int r,rx,ry,rt;
	if (parts[i].temp > 2273.15 && pv[y/CELL][x/CELL] > 50.0f)
		parts[i].tmp = 1;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				rt = (r&0xFF);
				if (!r)
					continue;
				if (pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL) // This will not work. DESL turns to fire above 5.0 pressure
				{
					part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					part_change_type(i,x,y,PT_OIL);
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
						create_part(i,x,y,PT_FIRE);
						parts[i].temp+=(rand()/(RAND_MAX/100));
						parts[i].tmp |= 1;
					}
				}
			}
	if (parts[i].temp > 2273.15 && pv[y/CELL][x/CELL] > 50.0f)
	{
		if (rand()%5 < 1)
		{
			int j;
			float temp = parts[i].temp;
			part_change_type(i,x,y,PT_PLSM);
			parts[i].life = rand()%150+50;
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1) parts[j].temp = temp;
			if (!(rand()%10)) { j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC); if (j != -1) parts[j].temp = temp; }
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT);
			if (j != -1) { parts[j].ctype = 0xFFFF00; parts[j].temp = temp; }

			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NBLE); if (j != -1) parts[j].temp = temp;

			if (rand()%2)
			{
				j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NBLE);
				if (j != -1) { parts[j].tmp = 1; parts[j].temp = temp; }
			}

			parts[i].temp += 750+rand()%500;
			pv[y/CELL][x/CELL] += 30;
		}
	}
	return 0;
}
