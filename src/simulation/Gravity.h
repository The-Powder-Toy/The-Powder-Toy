#pragma once
#include "Config.h"
#include "GravityPtr.h"
#include "SimulationConfig.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

class Simulation;

class Gravity
{
protected:
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

	struct mask_el {
		char *shape;
		char shapeout;
		mask_el *next;
	};
	using mask_el = struct mask_el;

	bool grav_mask_r(int x, int y, char checkmap[YCELLS][XCELLS], char shape[YCELLS][XCELLS]);
	void mask_free(mask_el *c_mask_el);

	void update_grav();
	void get_result();
	void update_grav_async();
	
	struct CtorTag // Please use Gravity::Create().
	{
	};

public:
	Gravity(CtorTag);
	~Gravity();

	//Maps to be used by the main thread
	float *gravmap = nullptr;
	float *gravp = nullptr;
	float *gravy = nullptr;
	float *gravx = nullptr;
	unsigned *gravmask = nullptr;

	unsigned char (*bmap)[XCELLS];

	bool IsEnabled() { return enabled; }

	void Clear();

	void gravity_update_async();

	void start_grav_async();
	void stop_grav_async();
	void gravity_mask();

	static GravityPtr Create();
};
