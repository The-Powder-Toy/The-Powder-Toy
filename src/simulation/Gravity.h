#ifndef GRAVITY_H
#define GRAVITY_H
#include "Config.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef GRAVFFT
#include <fftw3.h>
#endif

class Simulation;

class Gravity
{
private:

	bool enabled = false;

	// Maps to be processed by the gravity thread
	float *th_ogravmap = nullptr;
	float *th_gravmap = nullptr;
	float *th_gravx = nullptr;
	float *th_gravy = nullptr;
	float *th_gravp = nullptr;

	int th_gravchanged = 0;

	std::thread gravthread;
	std::mutex gravmutex;
	std::condition_variable gravcv;
	int grav_ready = 0;
	int gravthread_done = 0;
	bool ignoreNextResult = false;

#ifdef GRAVFFT
	bool grav_fft_status = false;
	float *th_ptgravx = nullptr;
	float *th_ptgravy = nullptr;
	float *th_gravmapbig = nullptr;
	float *th_gravxbig = nullptr;
	float *th_gravybig = nullptr;

	fftwf_complex *th_ptgravxt, *th_ptgravyt, *th_gravmapbigt, *th_gravxbigt, *th_gravybigt;
	fftwf_plan plan_gravmap, plan_gravx_inverse, plan_gravy_inverse;
#endif

	struct mask_el {
		char *shape;
		char shapeout;
		mask_el *next;
	};
	using mask_el = struct mask_el;

	bool grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL]);
	void mask_free(mask_el *c_mask_el);

	void update_grav();
	void update_grav_async();


#ifdef GRAVFFT
	void grav_fft_init();
	void grav_fft_cleanup();
#endif

public:
	//Maps to be used by the main thread
	float *gravmap = nullptr;
	float *gravp = nullptr;
	float *gravy = nullptr;
	float *gravx = nullptr;
	unsigned *gravmask = nullptr;

	unsigned char (*bmap)[XRES/CELL];

	bool IsEnabled() { return enabled; }

	void Clear();

	void gravity_update_async();

	void start_grav_async();
	void stop_grav_async();
	void gravity_mask();

	Gravity();
	~Gravity();
};

#endif
