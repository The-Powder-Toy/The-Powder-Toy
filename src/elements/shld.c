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

int update_SHLD1(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
				{
					if (55>rand()%200&&parts[i].life==0)
					{
						part_change_type(i,x,y,PT_SHLD2);
						parts[i].life = 7;
					}
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								//parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
							}
						}
				}
				else if ((r&0xFF)==PT_SHLD3&&4>rand()%10)
				{
					part_change_type(i,x,y,PT_SHLD2);
					parts[i].life = 7;
				}
			}
	return 0;
}

int update_SHLD2(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r && parts[i].life>0)
					create_part(-1,x+rx,y+ry,PT_SHLD1);
				if (!r)
					continue;
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
				{
					if (25>rand()%200&&parts[i].life==0)
					{
						part_change_type(i,x,y,PT_SHLD3);
						parts[i].life = 7;
					}
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								np = create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
				}
				else if ((r&0xFF)==PT_SHLD4&&4>rand()%10)
				{
					part_change_type(i,x,y,PT_SHLD3);
					parts[i].life = 7;
				}
			}
	return 0;
}

int update_SHLD3(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (1>rand()%2500)
					{
						np = create_part(-1,x+rx,y+ry,PT_SHLD1);
						if (np<0) continue;
						parts[np].life=7;
						part_change_type(i,x,y,PT_SHLD2);
					}
					else
						continue;

				}
				if ((r&0xFF)==PT_SHLD1 && parts[i].life>3)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_SHLD2);
					parts[r>>8].life=7;
				}
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
				{
					if (18>rand()%3000&&parts[i].life==0)
					{
						part_change_type(i,x,y,PT_SHLD4);
						parts[i].life = 7;
					}
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{

							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								np = create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
				}
			}
	return 0;
}

int update_SHLD4(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry, np;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (1>rand()%5500)
					{
						np = create_part(-1,x+rx,y+ry,PT_SHLD1);
						if (np<0) continue;
						parts[np].life=7;
						part_change_type(i,x,y,PT_SHLD2);
					}
					else
						continue;

				}
				if ((r&0xFF)==PT_SHLD2 && parts[i].life>3)
				{
					part_change_type(r>>8,x+rx,y+ry,PT_SHLD3);
					parts[r>>8].life = 7;
				}
				else if ((r&0xFF)==PT_SPRK&&parts[i].life==0)
					for ( nnx=-1; nnx<2; nnx++)
						for ( nny=-1; nny<2; nny++)
						{
							if (!pmap[y+ry+nny][x+rx+nnx])
							{
								np = create_part(-1,x+rx+nnx,y+ry+nny,PT_SHLD1);
								if (np<0) continue;
								parts[np].life=7;
							}
						}
			}
	return 0;
}
