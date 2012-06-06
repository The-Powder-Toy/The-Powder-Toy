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

int update_EXOT(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry, nb, rrx, rry, trade, tym, t;
	t = parts[i].type;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES) {
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WARP)
				{
					if (parts[r>>8].tmp2>2000)
						if (1>rand()%100)
						{
							parts[i].tmp2 += 100;
						}
				}
			}
	parts[i].tmp--;	
	parts[i].tmp2--;	
	if (parts[i].tmp<1 || parts[i].tmp>250) 
		parts[i].tmp = 250;
	if (parts[i].tmp2<1)
		parts[i].tmp2 = 1;
	else if (parts[i].tmp2>6000)
	{
		parts[i].tmp2 = 6000;
		part_change_type(i, x, y, PT_WARP);
		parts[i].temp = 10000;
	}
	else
		pv[y/CELL][x/CELL] += (parts[i].tmp2*CFDS)/160000;
	if (pv[y/CELL][x/CELL]>200 && parts[i].temp>9000 && parts[i].tmp2>200)
	{
		part_change_type(i, x, y, PT_WARP);
		parts[i].tmp2 = 6000;
	}		
	if (parts[i].tmp2>100)
	{
		for ( trade = 0; trade<9; trade ++)
		{
			rx = rand()%5-2;
			ry = rand()%5-2;
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==t && (parts[i].tmp2>parts[r>>8].tmp2) && parts[r>>8].tmp2>=0 )//diffusion
				{
					tym = parts[i].tmp2 - parts[r>>8].tmp2;
					if (tym ==1)
					{
						parts[r>>8].tmp2 ++;
						parts[i].tmp2 --;
						break;
					}
					if (tym>0)
					{
						parts[r>>8].tmp2 += tym/2;
						parts[i].tmp2 -= tym/2;
						break;
					}
				}
			}
		}
	}
	return 0;
}
int graphics_EXOT(GRAPHICS_FUNC_ARGS)
{
	int q = cpart->temp;
	int b = cpart->tmp;
	if ((cpart->tmp2 - 1)>rand()%1000)
	{	
	float frequency = 0.90045;	
	*colr = (sin(frequency*q + 0) * 127 + 255);
	*colg = (sin(frequency*q + 2) * 127 + 255);
	*colb = (sin(frequency*q + 4) * 127 + 255);
	*firea = 100;
	*firer = 0;
	*fireg = 0;
	*fireb = 0;
	*pixel_mode |= PMODE_FLAT;
//	*pixel_mode |= FIRE_ADD;
	*pixel_mode |= PMODE_FLARE;
	}
	else
	{
	float frequency = 0.00045;	
	*colr = (sin(frequency*q + 0) * 127 + (b/1.7));
	*colg = (sin(frequency*q + 2) * 127 + (b/1.7));
	*colb = (sin(frequency*q + 4) * 127 + (b/1.7));
	*cola = cpart->tmp / 6;	
	*firea = *cola;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	*pixel_mode |= FIRE_ADD;
	*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}
