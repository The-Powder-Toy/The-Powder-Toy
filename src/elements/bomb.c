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

int update_BOMB(UPDATE_FUNC_ARGS) {
	int r, rx, ry, nb;
	//Spark is used so much now that it should be a seperate element.
	if (parts[i].tmp==1) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (ptypes[r&0xFF].properties & (TYPE_SOLID | TYPE_PART | TYPE_LIQUID) && !(ptypes[r&0xFF].properties & PROP_SPARKSETTLE)) {
						kill_part(i);
						return 1;
					}
				}
	} else if (parts[i].tmp==0) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_BOMB && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_BCLN) {
						int rad = 8;
						int nxi;
						int nxj;
						pmap[y][x] = 0;
						for (nxj=-rad; nxj<=rad; nxj++)
							for (nxi=-rad; nxi<=rad; nxi++)
								if ((pow(nxi,2))/(pow(rad,2))+(pow(nxj,2))/(pow(rad,2))<=1)
									if ((pmap[y+nxj][x+nxi]&0xFF)!=PT_DMND && (pmap[y+nxj][x+nxi]&0xFF)!=PT_CLNE && (pmap[y+nxj][x+nxi]&0xFF)!=PT_PCLN && (pmap[y+nxj][x+nxi]&0xFF)!=PT_BCLN) {
										delete_part(x+nxi, y+nxj, 0);//it SHOULD kill anything but the exceptions above, doesn't seem to always work
										pv[(y+nxj)/CELL][(x+nxi)/CELL] += 0.1f;
										nb = create_part(-1, x+nxi, y+nxj, PT_BOMB);
										if (nb!=-1) {
											parts[nb].tmp = 2;
											parts[nb].life = 2;
											parts[nb].temp = MAX_TEMP;
										}
									}
						for (nxj=-(rad+1); nxj<=(rad+1); nxj++)
							for (nxi=-(rad+1); nxi<=(rad+1); nxi++)
								if ((pow(nxi,2))/(pow((rad+1),2))+(pow(nxj,2))/(pow((rad+1),2))<=1) {
									nb = create_part(-1, x+nxi, y+nxj, PT_BOMB);
									if (nb!=-1) {
										parts[nb].tmp = 1;
										parts[nb].life = 50;
										parts[nb].temp = MAX_TEMP;
										parts[nb].vx = rand()%20-10;
										parts[nb].vy = rand()%20-10;
									}
								}
						//create_parts(x, y, 9, 9, PT_BOMB);
						//create_parts(x, y, 8, 8, PT_NONE);
						kill_part(i);
						return 1;
					}
				}
	}
	return 0;
}
int graphics_BOMB(GRAPHICS_FUNC_ARGS)
{
	if (cpart->tmp==0) {
		//Normal bomb
		*pixel_mode |= PMODE_FLARE;
	}
	else if(cpart->tmp==2)
	{
		//Flash
		*pixel_mode = PMODE_FLAT | FIRE_ADD;
		*colr = *colg = *colb = *firer = *fireg = *fireb = *firea = 255;
	}
	else
	{
		//Flying spark
		*pixel_mode = PMODE_SPARK | PMODE_ADD;
		*cola = 4*cpart->life;
	}
	return 0;
}
