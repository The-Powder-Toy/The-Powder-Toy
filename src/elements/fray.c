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

int update_FRAY(UPDATE_FUNC_ARGS) {
	int r, nxx, nyy, docontinue, len, nxi, nyi, rx, ry, nr, ry1, rx1;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK) {
					for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1, len = 0; docontinue; nyy+=nyi, nxx+=nxi, len++) {
						if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0) || len>10) {
							break;
						}
						r = pmap[y+nyi+nyy][x+nxi+nxx];
						if (!r)
							r = photons[y+nyi+nyy][x+nxi+nxx];
			
						if (r && !(ptypes[r&0xFF].properties & TYPE_SOLID)){
							parts[r>>8].vx += nxi*((parts[i].temp-273.15)/10.0f);
							parts[r>>8].vy += nyi*((parts[i].temp-273.15)/10.0f);
						}
					}
				}
			}
	return 0;
}
