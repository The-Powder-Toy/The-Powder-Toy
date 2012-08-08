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

int update_SPAWN2(UPDATE_FUNC_ARGS) {
	if (!player2.spwn)
		create_part(-1, x, y, PT_STKM2);

	return 0;
}

int update_STKM2(UPDATE_FUNC_ARGS) {
	run_stickman(&player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}