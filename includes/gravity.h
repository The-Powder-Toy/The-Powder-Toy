/**
 * Powder Toy - Newtonian gravity (header)
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
#ifndef GRAVITY_H
#define GRAVITY_H

#include "defines.h"

extern int ngrav_enable; //Newtonian gravity
extern int gravwl_timeout;
extern int gravityMode;

float *gravmap;//Maps to be used by the main thread
float *gravp;
float *gravy;
float *gravx;
unsigned *gravmask;

float *th_ogravmap;// Maps to be processed by the gravity thread
float *th_gravmap;
float *th_gravx;
float *th_gravy;
float *th_gravp;

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
