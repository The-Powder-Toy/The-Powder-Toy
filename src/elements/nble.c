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
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1) parts[j].temp = temp;
			if (!(rand()%25)) { j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC); if (j != -1) parts[j].temp = temp; }
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_PHOT);
			if (j != -1) { parts[j].ctype = 0xFF0000; parts[j].temp = temp; }

			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_CO2);
			if (j != -1) parts[j].temp = temp;

			parts[i].temp += 1750+rand()%500;
			pv[y/CELL][x/CELL] += 50;
		}
	}
	return 0;
}
