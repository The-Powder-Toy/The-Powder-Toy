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

int update_RIME(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	parts[i].vx = 0;
	parts[i].vy = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK)
				{
					part_change_type(i,x,y,PT_FOG);
					parts[i].life = rand()%50 + 60;
				}
				else if ((r&0xFF)==PT_FOG&&parts[r>>8].life>0)
				{
					part_change_type(i,x,y,PT_FOG);
					parts[i].life = parts[r>>8].life;
				}
			}
	return 0;
}
