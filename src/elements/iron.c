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

int update_IRON(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((((r&0xFF) == PT_SALT && 15>(rand()/(RAND_MAX/700))) ||
				        ((r&0xFF) == PT_SLTW && 30>(rand()/(RAND_MAX/2000))) ||
				        ((r&0xFF) == PT_WATR && 5 >(rand()/(RAND_MAX/6000))) ||
				        ((r&0xFF) == PT_O2   && 2 >(rand()/(RAND_MAX/500))) ||
				        ((r&0xFF) == PT_LO2))&&
				        (!(parts[i].life))
				   )
				{
					part_change_type(i,x,y,PT_BMTL);
					parts[i].tmp=(rand()/(RAND_MAX/10))+20;
				}
			}
	return 0;
}
