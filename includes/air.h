#ifndef AIR_H
#define AIR_H
#include "defines.h"

extern float gravmap[YRES/CELL][XRES/CELL]; //Maps to be used by the main thread
extern float gravx[YRES/CELL][XRES/CELL];
extern float gravy[YRES/CELL][XRES/CELL];
extern float gravp[YRES/CELL][XRES/CELL];
extern float *gravpf;
extern float *gravxf;
extern float *gravyf;
extern unsigned gravmask[YRES/CELL][XRES/CELL];

extern float th_ogravmap[YRES/CELL][XRES/CELL]; // Maps to be processed by the gravity thread
extern float th_gravmap[YRES/CELL][XRES/CELL];
extern float th_gravx[YRES/CELL][XRES/CELL];
extern float th_gravy[YRES/CELL][XRES/CELL];
extern float *th_gravpf;
extern float *th_gravxf;
extern float *th_gravyf;
extern float th_gravp[YRES/CELL][XRES/CELL];

extern float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
extern float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
extern float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];

extern float cb_vx[YRES/CELL][XRES/CELL];
extern float cb_vy[YRES/CELL][XRES/CELL];
extern float cb_pv[YRES/CELL][XRES/CELL];
extern float cb_hv[YRES/CELL][XRES/CELL];

extern float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];

extern float hv[YRES/CELL][XRES/CELL], ohv[YRES/CELL][XRES/CELL]; // Ambient Heat

extern float kernel[9];

void make_kernel(void);

void update_airh(void);

void update_grav(void);

#ifdef GRAVFFT
void grav_fft_init();
void grav_fft_cleanup();
#endif

void update_air(void);

#endif
