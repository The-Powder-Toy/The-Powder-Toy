#ifndef GRAVITY_H
#define GRAVITY_H

#include "defines.h"

extern int ngrav_enable; //Newtonian gravity
extern int gravwl_timeout;
extern int gravityMode;

extern float gravmap[YRES/CELL][XRES/CELL]; //Maps to be used by the main thread
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

void gravity_init();
void gravity_cleanup();
void gravity_update_async();

void start_grav_async();
void stop_grav_async();
void update_grav();
void gravity_mask();

void bilinear_interpolation(float *src, float *dst, int sw, int sh, int rw, int rh);

#ifdef GRAVFFT
void grav_fft_init();
void grav_fft_cleanup();
#endif

#endif
