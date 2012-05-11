/**
 * Powder Toy - air simulation (header)
 *
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
#ifndef AIR_H
#define AIR_H
#include "defines.h"

extern float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
extern float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
extern float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];
extern unsigned char bmap_blockair[YRES/CELL][XRES/CELL];
extern unsigned char bmap_blockairh[YRES/CELL][XRES/CELL];

extern float cb_vx[YRES/CELL][XRES/CELL];
extern float cb_vy[YRES/CELL][XRES/CELL];
extern float cb_pv[YRES/CELL][XRES/CELL];
extern float cb_hv[YRES/CELL][XRES/CELL];

extern float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];

extern float hv[YRES/CELL][XRES/CELL], ohv[YRES/CELL][XRES/CELL]; // Ambient Heat

extern float kernel[9];

void make_kernel(void);

void update_airh(void);

void update_air(void);

#endif
