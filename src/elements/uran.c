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

int update_URAN(UPDATE_FUNC_ARGS) {
	if (!legacy_enable && pv[y/CELL][x/CELL]>0.0f)
	{
		float atemp =  parts[i].temp + (-MIN_TEMP);
		parts[i].temp = restrict_flt((atemp*(1+(pv[y/CELL][x/CELL]/2000)))+MIN_TEMP, MIN_TEMP, MAX_TEMP);
	}
	return 0;
}
