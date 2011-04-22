#ifndef AIR_H
#define AIR_H
#include "defines.h"

extern float ogravmap[YRES/CELL][XRES/CELL];
extern float gravmap[YRES/CELL][XRES/CELL];
extern float gravx[YRES/CELL][XRES/CELL];
extern float gravy[YRES/CELL][XRES/CELL];

extern float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
extern float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
extern float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];

extern float cb_vx[YRES/CELL][XRES/CELL], cb_ovx[YRES/CELL][XRES/CELL];
extern float cb_vy[YRES/CELL][XRES/CELL], cb_ovy[YRES/CELL][XRES/CELL];
extern float cb_pv[YRES/CELL][XRES/CELL], cb_opv[YRES/CELL][XRES/CELL];

extern float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];

extern float kernel[9];

void make_kernel(void);

void update_grav(void);

void update_air(void);

#endif