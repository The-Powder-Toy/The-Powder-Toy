#include "Gravity.h"
#include "simulation/CoordStack.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"
#include "Misc.h"
#include <cmath>
#include <iostream>
#include <sys/types.h>

Gravity::Gravity(CtorTag)
{
	th_ogravmap.resize(NCELL);
	th_gravmap.resize(NCELL);
	th_gravy.resize(NCELL);
	th_gravx.resize(NCELL);
	th_gravp.resize(NCELL);
	gravmap.resize(NCELL);
	gravy.resize(NCELL);
	gravx.resize(NCELL);
	gravp.resize(NCELL);
	gravmask.resize(NCELL);
}

Gravity::~Gravity()
{
	stop_grav_async();
}

void Gravity::Clear()
{
	std::fill(&gravy[0], &gravy[0] + NCELL, 0.0f);
	std::fill(&gravx[0], &gravx[0] + NCELL, 0.0f);
	std::fill(&gravp[0], &gravp[0] + NCELL, 0.0f);
	std::fill(&gravmap[0], &gravmap[0] + NCELL, 0.0f);
	std::fill(&gravmask[0], &gravmask[0] + NCELL, UINT32_C(0xFFFFFFFF));

	ignoreNextResult = true;
}

void Gravity::gravity_update_async()
{
	int result;
	if (!enabled)
		return;

	bool signal_grav = false;

	{
		std::unique_lock<std::mutex> l(gravmutex, std::defer_lock);
		if (l.try_lock())
		{
			result = grav_ready;
			if (result) //Did the gravity thread finish?
			{
				if (th_gravchanged && !ignoreNextResult)
				{
					// Copy thread gravity maps into this one
					get_result();
				}
				ignoreNextResult = false;

				std::swap(gravmap, th_gravmap);

				grav_ready = 0; //Tell the other thread that we're ready for it to continue
				signal_grav = true;
			}
		}
	}

	if (signal_grav)
	{
		gravcv.notify_one();
	}
	unsigned int size = NCELL;
	membwand(&gravy[0], &gravmask[0], size * sizeof(float), size * sizeof(uint32_t));
	membwand(&gravx[0], &gravmask[0], size * sizeof(float), size * sizeof(uint32_t));
	std::fill(&gravmap[0], &gravmap[0] + size, 0.0f);
}

void Gravity::update_grav_async()
{
	int done = 0;
	int thread_done = 0;
	std::fill(&th_ogravmap[0], &th_ogravmap[0] + NCELL, 0.0f);
	std::fill(&th_gravmap[0], &th_gravmap[0] + NCELL, 0.0f);
	std::fill(&th_gravy[0], &th_gravy[0] + NCELL, 0.0f);
	std::fill(&th_gravx[0], &th_gravx[0] + NCELL, 0.0f);
	std::fill(&th_gravp[0], &th_gravp[0] + NCELL, 0.0f);

	std::unique_lock<std::mutex> l(gravmutex);
	while (!thread_done)
	{
		if (!done)
		{
			// run gravity update
			update_grav();
			done = 1;
			grav_ready = 1;
			thread_done = gravthread_done;
		}
		else
		{
			// wait for main thread
			gravcv.wait(l);
			done = grav_ready;
			thread_done = gravthread_done;
		}
	}
}

void Gravity::start_grav_async()
{
	if (enabled)	//If it's already enabled, restart it
		stop_grav_async();

	gravthread_done = 0;
	grav_ready = 0;
	gravthread = std::thread([this]() { update_grav_async(); }); //Start asynchronous gravity simulation
	enabled = true;

	std::fill(&gravy[0], &gravy[0] + NCELL, 0.0f);
	std::fill(&gravx[0], &gravx[0] + NCELL, 0.0f);
	std::fill(&gravp[0], &gravp[0] + NCELL, 0.0f);
	std::fill(&gravmap[0], &gravmap[0] + NCELL, 0.0f);
}

void Gravity::stop_grav_async()
{
	if (enabled)
	{
		{
			std::lock_guard<std::mutex> g(gravmutex);
			gravthread_done = 1;
		}
		gravcv.notify_one();
		gravthread.join();
		enabled = false;
	}
	// Clear the grav velocities
	std::fill(&gravy[0], &gravy[0] + NCELL, 0.0f);
	std::fill(&gravx[0], &gravx[0] + NCELL, 0.0f);
	std::fill(&gravp[0], &gravp[0] + NCELL, 0.0f);
	std::fill(&gravmap[0], &gravmap[0] + NCELL, 0.0f);
}

bool Gravity::grav_mask_r(int x, int y, char checkmap[YCELLS][XCELLS], char shape[YCELLS][XCELLS])
{
	int x1, x2;
	bool ret = false;
	try
	{
		CoordStack cs;
		cs.push(x, y);
		do
		{
			cs.pop(x, y);
			x1 = x2 = x;
			while (x1 >= 0)
			{
				if (x1 == 0)
				{
					ret = true;
					break;
				}
				else if (checkmap[y][x1-1] || bmap[y][x1-1] == WL_GRAV)
					break;
				x1--;
			}
			while (x2 <= XCELLS-1)
			{
				if (x2 == XCELLS-1)
				{
					ret = true;
					break;
				}
				else if (checkmap[y][x2+1] || bmap[y][x2+1] == WL_GRAV)
					break;
				x2++;
			}
			for (x = x1; x <= x2; x++)
			{
				shape[y][x] = 1;
				checkmap[y][x] = 1;
			}
			if (y == 0)
			{
				for (x = x1; x <= x2; x++)
					if (bmap[y][x] != WL_GRAV)
						ret = true;
			}
			else if (y >= 1)
			{
				for (x = x1; x <= x2; x++)
					if (!checkmap[y-1][x] && bmap[y-1][x] != WL_GRAV)
					{
						if (y-1 == 0)
							ret = true;
						cs.push(x, y-1);
					}
			}
			if (y < YCELLS-1)
				for (x=x1; x<=x2; x++)
					if (!checkmap[y+1][x] && bmap[y+1][x] != WL_GRAV)
					{
						if (y+1 == YCELLS-1)
							ret = true;
						cs.push(x, y+1);
					}
		} while (cs.getSize()>0);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		ret = false;
	}
	return ret;
}
void Gravity::mask_free(mask_el *c_mask_el)
{
	if (c_mask_el == nullptr)
		return;
	delete[] c_mask_el->next;
	delete[] c_mask_el->shape;
	delete[] c_mask_el;
}

void Gravity::gravity_mask()
{
	char checkmap[YCELLS][XCELLS];
	unsigned maskvalue;
	mask_el *t_mask_el = nullptr;
	mask_el *c_mask_el = nullptr;
	memset(checkmap, 0, sizeof(checkmap));
	for (int x = 0; x < XCELLS; x++)
	{
		for(int y = 0; y < YCELLS; y++)
		{
			if (bmap[y][x] != WL_GRAV && checkmap[y][x] == 0)
			{
				// Create a new shape
				if (t_mask_el == nullptr)
				{
					t_mask_el = new mask_el[sizeof(mask_el)];
					t_mask_el->shape = new char[NCELL];
					std::fill(&t_mask_el->shape[0], &t_mask_el->shape[0] + NCELL, 0);
					t_mask_el->shapeout = 0;
					t_mask_el->next = nullptr;
					c_mask_el = t_mask_el;
				}
				else
				{
					c_mask_el->next = new mask_el[sizeof(mask_el)];
					c_mask_el = c_mask_el->next;
					c_mask_el->shape = new char[NCELL];
					std::fill(&c_mask_el->shape[0], &c_mask_el->shape[0] + NCELL, 0);
					c_mask_el->shapeout = 0;
					c_mask_el->next = nullptr;
				}
				// Fill the shape
				if (grav_mask_r(x, y, checkmap, reinterpret_cast<char(*)[XCELLS]>(c_mask_el->shape)))
					c_mask_el->shapeout = 1;
			}
		}
	}
	c_mask_el = t_mask_el;
	std::fill(&gravmask[0], &gravmask[0] + NCELL, 0);
	while (c_mask_el != nullptr)
	{
		char *cshape = c_mask_el->shape;
		for (int x = 0; x < XCELLS; x++)
		{
			for (int y = 0; y < YCELLS; y++)
			{
				if (cshape[y * XCELLS + x])
				{
					if (c_mask_el->shapeout)
						maskvalue = 0xFFFFFFFF;
					else
						maskvalue = 0x00000000;
					gravmask[y * XCELLS + x] = maskvalue;
				}
			}
		}
		c_mask_el = c_mask_el->next;
	}
	mask_free(t_mask_el);
}
