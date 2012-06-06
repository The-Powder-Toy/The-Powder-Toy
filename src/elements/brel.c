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

int update_BREL(UPDATE_FUNC_ARGS) {
	int np;
	if (1>rand()%200 && (pv[y/CELL][x/CELL] > 30.0f) && parts[i].temp>9000 && parts[i].life>0)
	{
		part_change_type(i, x ,y ,PT_EXOT);
		parts[i].life = 1000;
	}
	if ((pv[y/CELL][x/CELL] > 10.0f) && (parts[i].life>0)) {
		parts[i].temp = parts[i].temp + (pv[y/CELL][x/CELL])/8;
	}
	return 0;
}

