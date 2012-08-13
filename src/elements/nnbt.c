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
    parts[i].life --;
    if(parts[i].life > 0) return 0;
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if (!r)
                    continue;

                if (((r&0xFF)==PT_BREL || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_BRMT || (r&0xFF)==PT_BMTL) &&
                    parts[r>>8].tmp > 0 && parts[r>>8].pavg[0] > 0 && parts[r>>8].pavg[1] > 0 && rand()%360==0)
                {
                    parts[r>>8].type = parts[r>>8].tmp;
                    parts[r>>8].x = parts[r>>8].pavg[0];
                    parts[r>>8].y = parts[r>>8].pavg[1];
                    parts[i].life += 200;
                }
            }
    return 0;
}

int graphics_NNBT(GRAPHICS_FUNC_ARGS)
{
    int col = cpart->life + 55;
    *colr = *colg = *colb = col;
	return 0;
}
