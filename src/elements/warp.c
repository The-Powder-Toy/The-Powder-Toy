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

int update_WARP(UPDATE_FUNC_ARGS) {
	int trade, r, rx, ry;

	if (parts[i].tmp2>2000)
	{
		parts[i].temp = 10000;
		pv[y/CELL][x/CELL] += (parts[i].tmp2/5000) * CFDS;
		if (2>rand()%100)
			create_part(-3, x, y, PT_ELEC);
	}
	for ( trade = 0; trade<5; trade ++)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)!=PT_WARP&&(r&0xFF)!=PT_STKM&&(r&0xFF)!=PT_STKM2&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_BCLN&&(r&0xFF)!=PT_PCLN)
			{
				parts[i].x = parts[r>>8].x;
				parts[i].y = parts[r>>8].y;
				parts[r>>8].x = x;
				parts[r>>8].y = y;
				parts[r>>8].vx = (rand()%4)-1.5;
		 		parts[r>>8].vy = (rand()%4)-2;
				parts[i].life += 4;
				pmap[y][x] = r;
				pmap[y+ry][x+rx] = (i<<8)|parts[i].type;
				trade = 5;
			}
		}
	}
	return 0;
}
int graphics_WARP(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode &= ~PMODE;
	return 0;
}
