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

int update_FOG(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (ptypes[r&0xFF].state==ST_SOLID&&5>=rand()%50&&parts[i].life==0&&!((r&0xFF)==PT_CLNE||(r&0xFF)==PT_PCLN)) // TODO: should this also exclude BCLN?
				{
					part_change_type(i,x,y,PT_RIME);
				}
				if ((r&0xFF)==PT_SPRK)
				{
					parts[i].life += rand()%20;
				}
			}
	return 0;
}
