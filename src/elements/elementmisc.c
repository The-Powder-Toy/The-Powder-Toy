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

int update_MISC(UPDATE_FUNC_ARGS) {
	int t = parts[i].type;
	if (t==PT_LOVE)
		ISLOVE=1;
	else if (t==PT_LOLZ)
		ISLOLZ=1;
	else if (t==PT_GRAV)
		ISGRAV=1;
	return 0;
}
