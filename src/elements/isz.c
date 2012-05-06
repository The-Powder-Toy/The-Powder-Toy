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

int update_ISZ(UPDATE_FUNC_ARGS) { // for both ISZS and ISOZ
	float rr, rrr;
	if (1>rand()%200 && ((int)(-4.0f*(pv[y/CELL][x/CELL])))>(rand()%1000))
	{
		create_part(i, x, y, PT_PHOT);
		rr = (rand()%228+128)/127.0f;
		rrr = (rand()%360)*3.14159f/180.0f;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
	}
	return 0;
}
