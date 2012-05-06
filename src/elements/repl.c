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

int update_REPL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, ri;
	for(ri = 0; ri <= 10; ri++)
	{
		rx = (rand()%20)-10;
		ry = (rand()%20)-10;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				r = photons[y+ry][x+rx];

			if (r && !(ptypes[r&0xFF].properties & TYPE_SOLID)){
				parts[r>>8].vx += isign(rx)*((parts[i].temp-273.15)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].temp-273.15)/10.0f);
			}
		}
	}
	return 0;
}
