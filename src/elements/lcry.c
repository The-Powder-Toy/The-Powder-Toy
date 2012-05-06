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

int update_LCRY(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	if(parts[i].tmp==1 || parts[i].tmp==0)
	{
		if(parts[i].tmp==1)
		{
			if(parts[i].life<=0)
				parts[i].tmp = 0;
			else
			{
				parts[i].life-=2;
				if(parts[i].life < 0)
					parts[i].life = 0;
				parts[i].tmp2 = parts[i].life;
			}	
		}
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_LCRY && parts[r>>8].tmp == 3)
					{
						parts[r>>8].tmp = 1;
					}
				}
	}
	else if(parts[i].tmp==2 || parts[i].tmp==3)
	{
		if(parts[i].tmp==2)
		{
			if(parts[i].life>=10)
				parts[i].tmp = 3;
			else
			{
				parts[i].life+=2;
				if(parts[i].life > 10)
					parts[i].life = 10;
				parts[i].tmp2 = parts[i].life;
			}
		}
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_LCRY && parts[r>>8].tmp == 0)
					{
						parts[r>>8].tmp = 2;
					}
				}
	}
	return 0;
}
