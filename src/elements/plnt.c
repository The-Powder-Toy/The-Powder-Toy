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

int update_PLNT(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>(rand()%250))
				{
					np = create_part(r>>8,x+rx,y+ry,PT_PLNT);
					if (np<0) continue;
					parts[np].life = 0;
				}
				else if ((r&0xFF)==PT_LAVA && 1>(rand()%250))
				{
					part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				else if (((r&0xFF)==PT_SMKE || (r&0xFF)==PT_CO2) && (1>rand()%250))
				{
					kill_part(r>>8);
					parts[i].life = rand()%60 + 60;
				}
				else if ((r&0xFF)==PT_WOOD && (1>rand()%20) && abs(rx+ry)<=2 && VINE_MODE)
				{
					int nnx = rand()%3 -1;
					int nny = rand()%3 -1;
					if (x+rx+nnx>=0 && y+ry+nny>0 && x+rx+nnx<XRES && y+ry+nny<YRES && (nnx || nny))
					{
						if (pmap[y+ry+nny][x+rx+nnx])
							continue;
						np = create_part(-1,x+rx+nnx,y+ry+nny,PT_VINE);
						if (np<0) continue;
						parts[np].temp = parts[i].temp;
					}
				}
			}
	if (parts[i].life==2)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						create_part(-1,x+rx,y+ry,PT_O2);
				}
		parts[i].life = 0;
	}
	return 0;
}
