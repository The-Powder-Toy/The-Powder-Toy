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
int update_GBMB(UPDATE_FUNC_ARGS) {
	int rx,ry,r;
	if (parts[i].life<=0)
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					continue;
				if((r&0xFF)!=PT_BOMB && (r&0xFF)!=PT_GBMB &&
				   (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN &&
				   (r&0xFF)!=PT_DMND)
				{
					parts[i].life=60;
					break;
				}
			}
	}
	if(parts[i].life>20)
		gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = 20;
	if(parts[i].life<20 && parts[i].life>=1)
		gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] = -80;
	return 0;
}
