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

int update_CO2(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
                if (20>(rand()%40000)&&parts[i].ctype==5)
				{
					parts[i].ctype = 0;
                    create_part(-3, x, y, PT_WATR);
				}
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_FIRE){
					kill_part(r>>8);
						if(1>(rand()%150)){
							kill_part(i);
							return 1;
						}
				}
				if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%250))
				{
					part_change_type(i,x,y,PT_CBNW);
					kill_part(r>>8);
				}
			}
	if (parts[i].temp > 9773.15 && pv[y/CELL][x/CELL] > 200.0f)
	{
		if (rand()%5 < 1)
		{
			int j;
			kill_part(i);
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_NEUT); if (j != -1) parts[j].temp = 15000;
			if (!(rand()%50)) { j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_ELEC); if (j != -1) parts[j].temp = 15000; }
			j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_O2);  if (j != -1) parts[j].temp = 15000;
			if (rand()%1000 < 1) { j = create_part(-3,x+rand()%3-1,y+rand()%3-1,PT_SING); if (j != -1) { parts[j].temp = 15000; parts[i].life = 3; } }

			parts[i].temp += 15000;
			pv[y/CELL][x/CELL] += 100;
			return 1;
		}
	}
	return 0;
}
