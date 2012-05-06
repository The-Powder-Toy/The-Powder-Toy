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

int update_QRTZ(UPDATE_FUNC_ARGS) {
	int r, tmp, trade, rx, ry, np, t;
	t = parts[i].type;
	if (t == PT_QRTZ)
	{
		parts[i].pavg[0] = parts[i].pavg[1];
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
		if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
		{
			part_change_type(i,x,y,PT_PQRT);
		}
	}
	// absorb SLTW
	if (parts[i].ctype!=-1)
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					else if ((r&0xFF)==PT_SLTW && (1>rand()%2500))
					{
						kill_part(r>>8);
						parts[i].ctype ++;
					}
				}
	// grow if absorbed SLTW
	if (parts[i].ctype>0)
	{
		for ( trade = 0; trade<5; trade ++)
		{
			rx = rand()%3-1;
			ry = rand()%3-1;
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r && parts[i].ctype!=0)
				{
					np = create_part(-1,x+rx,y+ry,PT_QRTZ);
					if (np>-1)
					{
						parts[np].tmp = parts[i].tmp;
						parts[i].ctype--;
						if (5>rand()%10)
						{
							parts[np].ctype=-1;//dead qrtz
						}
						else if (!parts[i].ctype && 1>rand()%15)
						{
							parts[i].ctype=-1;
						}

						break;
					}
				}
			}
		}
	}
	// diffuse absorbed SLTW
	if (parts[i].ctype>0)
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
				if ((r&0xFF)==t && (parts[i].ctype>parts[r>>8].ctype) && parts[r>>8].ctype>=0 )//diffusion
				{
					tmp = parts[i].ctype - parts[r>>8].ctype;
					if (tmp ==1)
					{
						parts[r>>8].ctype ++;
						parts[i].ctype --;
						break;
					}
					if (tmp>0)
					{
						parts[r>>8].ctype += tmp/2;
						parts[i].ctype -= tmp/2;
						break;
					}
				}
			}
		}
	}
	return 0;
}
