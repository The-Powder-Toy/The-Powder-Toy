#ifndef GRAVITY_H
#define GRAVITY_H

#include <pthread.h>
#include "Config.h"
#include "Simulation.h"

class Simulation;

struct mask_el {
	char *shape;
	char shapeout;
	void *next;
};
typedef struct mask_el mask_el;


/*
 * 	float *gravmap = NULL;//Maps to be used by the main thread
	float *gravp = NULL;
	float *gravy = NULL;
	float *gravx = NULL;
	unsigned *gravmask = NULL;

	float *th_ogravmap = NULL;// Maps to be processed by the gravity thread
	float *th_gravmap = NULL;
	float *th_gravx = NULL;
	float *th_gravy = NULL;
	float *th_gravp = NULL;

	int th_gravchanged = 0;

	pthread_t gravthread;
	pthread_mutex_t gravmutex;
	pthread_cond_t gravcv;
	int grav_ready = 0;
	int gravthread_done = 0;
 */
class Gravity
{
private:
	unsigned *gravmask;

	float *th_ogravmap;
	float *th_gravmap;
	float *th_gravx;
	float *th_gravy;
	float *th_gravp;

	int th_gravchanged;

	pthread_t gravthread;
	pthread_mutex_t gravmutex;
	pthread_cond_t gravcv;
	int grav_ready;
	int gravthread_done;

	//Simulation * sim;
public:
	float *gravmap;
	float *gravp;
	float *gravy;
	float *gravx;
	unsigned char (*bmap)[XRES/CELL];
	int ngrav_enable;
	void grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL], char *shapeout);
	void mask_free(mask_el *c_mask_el);

	void gravity_init();
	void gravity_cleanup();
	void gravity_update_async();

	static void *update_grav_async_helper(void * context);
	void update_grav_async();

	void start_grav_async();
	void stop_grav_async();
	void update_grav();
	void gravity_mask();

	void bilinear_interpolation(float *src, float *dst, int sw, int sh, int rw, int rh);

	#ifdef GRAVFFT
	void grav_fft_init();
	void grav_fft_cleanup();
	#endif

	Gravity();
};

/*extern int ngrav_enable; //Newtonian gravity
extern int gravwl_timeout;
extern int gravityMode;*/

/*float *gravmap;//Maps to be used by the main thread
float *gravp;
float *gravy;
float *gravx;
unsigned *gravmask;

float *th_ogravmap;// Maps to be processed by the gravity thread
float *th_gravmap;
float *th_gravx;
float *th_gravy;
float *th_gravp;*/


#endif
