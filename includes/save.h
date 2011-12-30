#ifndef SAVE_H
#define SAVE_H

void *build_thumb(int *size, int bzip2);

void *build_save(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

void *build_save_OPS(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save_OPS(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float vx[YRES/CELL][XRES/CELL], float vy[YRES/CELL][XRES/CELL], float pv[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

//Old save parsers, readers
void *build_save_PSv(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr);

int parse_save_PSv(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES]);

#endif