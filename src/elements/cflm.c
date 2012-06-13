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

int update_CFLM(UPDATE_FUNC_ARGS) {
	if (parts[i].life==1 && parts[i].tmp==5)
	{
		part_change_type(i, x, y, PT_LO2);
		parts[i].life = 0;
		parts[i].temp = 15;
		parts[i].tmp = 0;
	}
	return 0;
}
