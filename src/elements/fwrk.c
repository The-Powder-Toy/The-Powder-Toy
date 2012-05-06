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

int update_FWRK(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	if ((parts[i].temp>400&&(9+parts[i].temp/40)>rand()%100000&&parts[i].life==0&&!pmap[y-1][x])||parts[i].ctype==PT_DUST)
	{
		np = create_part(-1, x , y-1 , PT_FWRK);
		if (np!=-1)
		{
			parts[np].vy = rand()%8-22;
			parts[np].vx = rand()%20-rand()%20;
			parts[np].life=rand()%15+25;
			parts[np].dcolour = parts[i].dcolour;
			kill_part(i);
			return 1;
		}
	}
	if (parts[i].life>=45)
		parts[i].life=0;
	if ((parts[i].life<3&&parts[i].life>0)||(parts[i].vy>6&&parts[i].life>0))
	{
		int q = (rand()%255+1);
		int w = (rand()%255+1);
		int e = (rand()%255+1);
		for (rx=-1; rx<2; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					if (5>=rand()%8)
					{
						if (!pmap[y+ry][x+rx])
						{
							np = create_part(-1, x+rx, y+ry , PT_DUST);
							pv[y/CELL][x/CELL] += 2.00f*CFDS;
							if (np!=-1)
							{
								parts[np].vy = -(rand()%10-1);
								parts[np].vx = ((rand()%2)*2-1)*rand()%(5+5)+(parts[i].vx)*2 ;
								parts[np].life= rand()%37+18;
								parts[np].tmp=q;
								parts[np].tmp2=w;
								parts[np].ctype=e;
								parts[np].temp= rand()%20+6000;
								parts[np].dcolour = parts[i].dcolour;
							}
						}
					}
				}
		kill_part(i);
		return 1;
	}
	return 0;
}
