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

int update_NNBT(UPDATE_FUNC_ARGS)
{
    int r,rx,ry;
    if(parts[i].life > 0) {
        parts[i].life --;
        return 0;
    }
    if(parts[i].tmp2 > 1 && rand()%720 > parts[i].tmp2)
        kill_part(i);
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if (!r)
                    continue;

                if((r&0xFF)==PT_EMP) parts[i].ctype = PT_EMP;

                if (((r&0xFF)==PT_BREL || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_BRMT || (r&0xFF)==PT_BMTL) &&
                    parts[r>>8].tmp > 0 && parts[r>>8].pavg[0] > 0 && parts[r>>8].pavg[1] > 0 && rand()%360==0 && parts[i].ctype != PT_EMP)
                {
                    parts[r>>8].type = parts[r>>8].tmp;
                    parts[r>>8].x = parts[r>>8].pavg[0];
                    parts[r>>8].y = parts[r>>8].pavg[1];
                    parts[i].life += 200;
                    parts[i].tmp2 ++;
                }

                if(parts[i].ctype == PT_EMP && ((r&0xFF)==PT_BREL || (r&0xFF)==PT_PSCN || (r&0xFF)==PT_NSCN || (r&0xFF)==PT_BMTL
                    || (r&0xFF)==PT_BRMT || (r&0xFF)==PT_METL || (r&0xFF)==PT_WIRE || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_PTCT) && rand()%16 == 0) {
				    parts[r>>8].tmp = parts[r>>8].type;
				    parts[r>>8].pavg[0] = parts[r>>8].x;
					parts[r>>8].pavg[1] = parts[r>>8].y;
					part_change_type(r>>8, rx, ry, PT_BREL);
                }
            }
    return 0;
}

int graphics_NNBT(GRAPHICS_FUNC_ARGS)
{
    int col = cpart->life + 55;
    *colr = *colg = *colb = *cola = col;
    if(cpart->life > 100) {
        *pixel_mode |= PMODE_FLARE;
    }
	return 0;
}
