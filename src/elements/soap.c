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

void attach(int i1, int i2)
{
	if (!(parts[i2].ctype&4))
	{
		parts[i1].ctype |= 2;
		parts[i1].tmp = i2;

		parts[i2].ctype |= 4;
		parts[i2].tmp2 = i1;
	}
	else
	if (!(parts[i2].ctype&2))
	{
		parts[i1].ctype |= 4;
		parts[i1].tmp2= i2;

		parts[i2].ctype |= 2;
		parts[i2].tmp = i1;
	}
}

int update_SOAP(UPDATE_FUNC_ARGS) 
{
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta;
	float blend;
	
	//0x01 - bubble on/off
	//0x02 - first mate yes/no
	//0x04 - "back" mate yes/no

	if (parts[i].ctype&1)
	{
		if (parts[i].temp>0)
		{
			if (parts[i].life<=0)
			{
				if ((parts[i].ctype&6) != 6 && (parts[i].ctype&6))
				{
					int target;

					target = i;

					while((parts[target].ctype&6) != 6 && (parts[target].ctype&6))
					{
						if (parts[target].ctype&2)
						{
							target = parts[target].tmp;
							detach(target);
						}

						if (parts[target].ctype&4)
						{
							target = parts[target].tmp2;
							detach(target);
						}
					}
				}

				if ((parts[i].ctype&6) != 6)
					parts[i].ctype = 0;

				if ((parts[i].ctype&6) == 6 && (parts[parts[i].tmp].ctype&6) == 6 && parts[parts[i].tmp].tmp == i)
					detach(i);
			}

			parts[i].vy -= 0.1f;

			parts[i].vy *= 0.5f;
			parts[i].vx *= 0.5f;
		}

		if (!(parts[i].ctype&2))
		{
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;

						if ((parts[r>>8].type == PT_SOAP) && (parts[r>>8].ctype&1) && !(parts[r>>8].ctype&4))
							attach(i, r>>8);
					}
		}
		else
		{
			if (parts[i].life<=0)
				for (rx=-2; rx<3; rx++)
					for (ry=-2; ry<3; ry++)
						if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
						{
							r = pmap[y+ry][x+rx];
							if (!r && !bmap[(y+ry)/CELL][(x+rx)/CELL])
								continue;

							if (parts[i].temp>0)
							{
								if (bmap[(y+ry)/CELL][(x+rx)/CELL] 
										|| (r && ptypes[r&0xFF].state != ST_GAS 
											&& (r&0xFF) != PT_SOAP && (r&0xFF) != PT_GLAS))
								{
									detach(i);
									continue;
								}
							}

							if ((r&0xFF) == PT_SOAP && parts[r>>8].ctype == 1)
							{
								int buf;

								buf = parts[i].tmp;

								parts[i].tmp = r>>8;
								parts[buf].tmp2 = r>>8;
								parts[r>>8].tmp2 = i;
								parts[r>>8].tmp = buf;
								parts[r>>8].ctype = 7;
							}

							if ((r&0xFF) == PT_SOAP && parts[r>>8].ctype == 7 && parts[i].tmp != r>>8 && parts[i].tmp2 != r>>8)
							{
								int buf;

								parts[parts[i].tmp].tmp2 = parts[r>>8].tmp2;
								parts[parts[r>>8].tmp2].tmp = parts[i].tmp;
								parts[r>>8].tmp2 = i;
								parts[i].tmp = r>>8;
							}
						}
		}

		if(parts[i].ctype&2)
		{
			float d, dx, dy;

			dx = parts[i].x - parts[parts[i].tmp].x;
			dy = parts[i].y - parts[parts[i].tmp].y;

			d = 9/(pow(dx, 2)+pow(dy, 2)+9)-0.5;

			parts[parts[i].tmp].vx -= dx*d;
			parts[parts[i].tmp].vy -= dy*d;

			parts[i].vx += dx*d;
			parts[i].vy += dy*d;

			if ((parts[parts[i].tmp].ctype&2) && (parts[parts[i].tmp].ctype&1) 
					&& (parts[parts[parts[i].tmp].tmp].ctype&2) && (parts[parts[parts[i].tmp].tmp].ctype&1))
			{
				int ii;

				ii = parts[parts[parts[i].tmp].tmp].tmp;

				dx = parts[ii].x - parts[parts[i].tmp].x;
				dy = parts[ii].y - parts[parts[i].tmp].y;

				d = 81/(pow(dx, 2)+pow(dy, 2)+81)-0.5;

				parts[parts[i].tmp].vx -= dx*d*0.5f;
				parts[parts[i].tmp].vy -= dy*d*0.5f;

				parts[ii].vx += dx*d*0.5f;
				parts[ii].vy += dy*d*0.5f;
			}
		}
	}
	else
	{
		if (pv[y/CELL][x/CELL]>0.5f || pv[y/CELL][x/CELL]<(-0.5f))
		{
			parts[i].ctype = 1;
			parts[i].life = 10;
		}

		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;

					if ((r&0xFF) == PT_OIL)
					{
						float ax, ay;

						parts[i].vy -= 0.1f;

						parts[i].vy *= 0.5f;
						parts[i].vx *= 0.5f;

						ax = (parts[i].vx + parts[r>>8].vx)/2;
						ay = (parts[i].vy + parts[r>>8].vy)/2;

						parts[i].vx = ax;
						parts[i].vy = ay;
						parts[r>>8].vx = ax;
						parts[r>>8].vy = ay;
					}
				}
	}
	
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_SOAP)
				{
					blend = 0.85f;
					tr = (parts[r>>8].dcolour>>16)&0xFF;
					tg = (parts[r>>8].dcolour>>8)&0xFF;
					tb = (parts[r>>8].dcolour)&0xFF;
					ta = (parts[r>>8].dcolour>>24)&0xFF;
					
					nr = (tr*blend);
					ng = (tg*blend);
					nb = (tb*blend);
					na = (ta*blend);
					
					parts[r>>8].dcolour = nr<<16 | ng<<8 | nb | na<<24;
				}
			}

	return 0;
}
