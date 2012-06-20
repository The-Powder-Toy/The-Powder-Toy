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
		int r = (rand()%245+11);
		int g = (rand()%245+11);
		int b = (rand()%245+11);
		int n;
		float angle, magnitude;
		unsigned col = (r<<16) | (g<<8) | b;
		for (n=0; n<40; n++)
		{
			np = create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = ((rand()%60)+40)*0.05f;
				angle = (rand()%6284)*0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy + sinf(angle)*magnitude - 2.5f;
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
