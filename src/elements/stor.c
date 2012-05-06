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

int update_STOR(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np, rx1, ry1;
	if(parts[i].life && !parts[i].tmp)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (!parts[i].tmp && !parts[i].life && (r&0xFF)!=PT_STOR && !(ptypes[(r&0xFF)].properties&TYPE_SOLID) && (!parts[i].ctype || (r&0xFF)==parts[i].ctype))
				{
					parts[i].tmp = parts[r>>8].type;
					parts[i].temp = parts[r>>8].temp;
					parts[i].tmp2 = parts[r>>8].life;
					parts[i].pavg[0] = parts[r>>8].tmp;
					parts[i].pavg[1] = parts[r>>8].ctype;
					kill_part(r>>8);
				}
				if(parts[i].tmp && (r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_PSCN && parts[r>>8].life>0 && parts[r>>8].life<4)
				{
					for(ry1 = 1; ry1 >= -1; ry1--){
						for(rx1 = 0; rx1 >= -1 && rx1 <= 1; rx1 = -rx1-rx1+1){ // Oscilate the X starting at 0, 1, -1, 3, -5, etc (Though stop at -1)
							np = create_part(-1,x+rx1,y+ry1,parts[i].tmp);
							if (np!=-1)
							{
								parts[np].temp = parts[i].temp;
								parts[np].life = parts[i].tmp2;
								parts[np].tmp = parts[i].pavg[0];
								parts[np].ctype = parts[i].pavg[1];
								parts[i].tmp = 0;
								parts[i].life = 10;
								break;
							}
						}
					}
				}
			}
	return 0;
}
