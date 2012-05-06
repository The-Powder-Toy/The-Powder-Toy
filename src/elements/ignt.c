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

int update_IGNT(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if(parts[i].tmp==0)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM)
					{
						parts[i].tmp = 1;
					}
					else if ((r&0xFF)==PT_SPRK || (r&0xFF)==PT_LIGH || ((r&0xFF)==PT_IGNT && parts[r>>8].life==1))
					{
						parts[i].tmp = 1;
					}
				}
	}
	else if(parts[i].life > 0)
	{
		if(rand()%3)
		{
			int nb = create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_BOMB);
			if (nb!=-1) {
				parts[nb].tmp = 1;
				parts[nb].life = 30;
				parts[nb].vx = rand()%20-10;
				parts[nb].vy = rand()%20-10;
				parts[nb].temp = restrict_flt(400.0f+parts[i].temp-273.15, MIN_TEMP, MAX_TEMP);
			}
		}
		else
		{
			create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
		}
		parts[i].life--;
	}
	return 0;
}
