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

int update_GEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float dx, dy;
	char gel;
	if (parts[i].tmp>100) parts[i].tmp = 100;
	if (parts[i].tmp<0) parts[i].tmp = 0;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				//Desaturation
				if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW || (r&0xFF)==PT_CBNW)
				    && parts[i].tmp<100)
				{
					parts[i].tmp++;
					kill_part(r>>8);
				}

				if ((r&0xFF)==PT_SPNG && parts[i].tmp<100 && ((parts[r>>8].life+1)>parts[i].tmp))
				{
					parts[r>>8].life--;
					parts[i].tmp++;
				}

				gel = 0;
				if ((r&0xFF)==PT_GEL)
					gel = 1;

				//Concentration diffusion
				if (gel && (parts[r>>8].tmp+1)<parts[i].tmp)
				{
					parts[r>>8].tmp++;
					parts[i].tmp--;
				}

				if ((r&0xFF)==PT_SPNG && (parts[r>>8].life+1)<parts[i].tmp)
				{
					parts[r>>8].life++;
					parts[i].tmp--;
				}

				dx = parts[i].x - parts[r>>8].x;
				dy = parts[i].y - parts[r>>8].y;

				//Stickness
				if ((dx*dx + dy*dy)>1.5 && (gel || !ptypes[r&0xFF].falldown || (fabs(rx)<2 && fabs(ry)<2)))
				{
					float per, nd;
					nd = dx*dx + dy*dy - 0.5;

					per = 5*(1 - parts[i].tmp/100)*(nd/(dx*dx + dy*dy + nd) - 0.5);
					if (ptypes[r&0xFF].state==ST_LIQUID)
						per *= 0.1;
					
					dx *= per; dy *= per;
					parts[i].vx += dx; 
					parts[i].vy += dy; 

					if (ptypes[r&0xFF].properties&TYPE_PART || (r&0xFF)==PT_GOO)
					{
						parts[r>>8].vx -= dx;
						parts[r>>8].vy -= dy;
					}
				}
			}
	return 0;
}
