/**
 * Powder Toy - saving and loading functions header
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
#ifndef SAVE_H
#define SAVE_H

void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate);

void *build_thumb(int *size, int bzip2);

pixel *prerender_save(void *save, int size, int *width, int *height);

void *build_save(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

pixel *prerender_save_OPS(void *save, int size, int *width, int *height);

void *build_save_OPS(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save_OPS(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

//Old save parsers, readers
pixel *prerender_save_PSv(void *save, int size, int *width, int *height);

void *build_save_PSv(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save_PSv(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

#endif
