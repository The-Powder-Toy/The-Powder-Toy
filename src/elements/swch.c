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

int update_SWCH(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (parts_avg(i,r>>8,PT_INSL)!=PT_INSL) {
					rt = r&0xFF;
					if (rt==PT_SWCH)
					{
						if (parts[i].life>=10&&parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life>=10)
						{
							//Set to other particle's life instead of 10, otherwise spark loops form when SWCH is sparked while turning on
							parts[i].life = parts[r>>8].life;
						}
					}
					else if (rt==PT_SPRK&&parts[i].life==10&&parts[r>>8].ctype!=PT_PSCN&&parts[r>>8].ctype!=PT_NSCN) {
						part_change_type(i,x,y,PT_SPRK);
						parts[i].ctype = PT_SWCH;
						parts[i].life = 4;
					}
				}
			}
	//turn off SWCH from two red BRAYS
	if (parts[i].life==10 && (!(pmap[y-1][x-1]&0xFF) && ((pmap[y-1][x]&0xFF)==PT_BRAY&&parts[pmap[y-1][x]>>8].tmp==2) && !(pmap[y-1][x+1]&0xFF) && ((pmap[y][x+1]&0xFF)==PT_BRAY&&parts[pmap[y][x+1]>>8].tmp==2)))
	{
		parts[i].life = 9;
	}
	//turn on SWCH from two red BRAYS
	else if (parts[i].life<=5 && (!(pmap[y-1][x-1]&0xFF) && (((pmap[y-1][x]&0xFF)==PT_BRAY&&parts[pmap[y-1][x]>>8].tmp==2) || ((pmap[y+1][x]&0xFF)==PT_BRAY&&parts[pmap[y+1][x]>>8].tmp==2)) && !(pmap[y-1][x+1]&0xFF) && (((pmap[y][x+1]&0xFF)==PT_BRAY&&parts[pmap[y][x+1]>>8].tmp==2) || ((pmap[y][x-1]&0xFF)==PT_BRAY&&parts[pmap[y][x-1]>>8].tmp==2))))
	{
		parts[i].life = 14;
	}
	return 0;
}
