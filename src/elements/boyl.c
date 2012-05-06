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

int update_BOYL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (pv[y/CELL][x/CELL]<(parts[i].temp/100))
		pv[y/CELL][x/CELL] += 0.001f*((parts[i].temp/100)-pv[y/CELL][x/CELL]);
	if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp/100))
		pv[y/CELL+1][x/CELL] += 0.001f*((parts[i].temp/100)-pv[y/CELL+1][x/CELL]);
	if (x+CELL<XRES)
	{
		pv[y/CELL][x/CELL+1] += 0.001f*((parts[i].temp/100)-pv[y/CELL][x/CELL+1]);
		if (y+CELL<YRES)
			pv[y/CELL+1][x/CELL+1] += 0.001f*((parts[i].temp/100)-pv[y/CELL+1][x/CELL+1]);
	}
	if (y-CELL>=0 && pv[y/CELL-1][x/CELL]<(parts[i].temp/100))
		pv[y/CELL-1][x/CELL] += 0.001f*((parts[i].temp/100)-pv[y/CELL-1][x/CELL]);
	if (x-CELL>=0)
	{
		pv[y/CELL][x/CELL-1] += 0.001f*((parts[i].temp/100)-pv[y/CELL][x/CELL-1]);
		if (y-CELL>=0)
			pv[y/CELL-1][x/CELL-1] += 0.001f*((parts[i].temp/100)-pv[y/CELL-1][x/CELL-1]);
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 &&
			        x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>rand()%30)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_FOG);
				}
				else if ((r&0xFF)==PT_O2 && 1>rand()%9)
				{
					kill_part(r>>8);
					part_change_type(i,x,y,PT_WATR);
					pv[y/CELL][x/CELL] += 4.0;
				}
			}
	return 0;
}
