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
#include "hmap.h"

int update_FIRW(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt, np;
	if (parts[i].tmp<=0) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = parts[r>>8].type;
					if (rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
					{
						float gx, gy, multiplier;
						get_gravity_field(x, y, ptypes[PT_FIRW].gravity, 1.0f, &gx, &gy);
						if (gx*gx+gy*gy < 0.001f)
						{
							float angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
							gx += sinf(angle)*ptypes[PT_FIRW].gravity*0.5f;
							gy += cosf(angle)*ptypes[PT_FIRW].gravity*0.5f;
						}
						parts[i].tmp = 1;
						parts[i].life = rand()%10+20;
						multiplier = (parts[i].life+20)*0.2f/sqrtf(gx*gx+gy*gy);
						parts[i].vx -= gx*multiplier;
						parts[i].vy -= gy*multiplier;
						return 0;
					}
				}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life<=0) {
			parts[i].tmp=2;
		} else {
			parts[i].flags &= ~FLAG_STAGNANT;
		}
	}
	else if (parts[i].tmp>=2) {
		float angle, magnitude;
		int caddress = (rand()%200)*3;
		int n;
		unsigned col = (((unsigned char)(firw_data[caddress]))<<16) | (((unsigned char)(firw_data[caddress+1]))<<8) | ((unsigned char)(firw_data[caddress+2]));
		for (n=0; n<40; n++)
		{
			np = create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = ((rand()%60)+40)*0.05f;
				angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx*0.5f + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy*0.5f + sinf(angle)*magnitude;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = rand()%40+70;
				parts[np].temp = (rand()%500)+5750.0f;
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		pv[y/CELL][x/CELL] += 8.0f;
		kill_part(i);
		return 1;
	}
	return 0;
}
