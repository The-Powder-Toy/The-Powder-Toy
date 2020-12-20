#include "Gravity.h"

#include <cmath>
#include <iostream>
#include <sys/types.h>

#include "CoordStack.h"
#include "Misc.h"
#include "Simulation.h"
#include "SimulationData.h"


Gravity::Gravity()
{
	// Allocate full size Gravmaps
	unsigned int size = (XRES / CELL) * (YRES / CELL);
	th_ogravmap = new float[size];
	th_gravmap = new float[size];
	th_gravy = new float[size];
	th_gravx = new float[size];
	th_gravp = new float[size];
	gravmap = new float[size];
	gravy = new float[size];
	gravx = new float[size];
	gravp = new float[size];
	gravmask = new unsigned[size];
}

Gravity::~Gravity()
{
	stop_grav_async();
#ifdef GRAVFFT
	grav_fft_cleanup();
#endif

	delete[] th_ogravmap;
	delete[] th_gravmap;
	delete[] th_gravy;
	delete[] th_gravx;
	delete[] th_gravp;
	delete[] gravmap;
	delete[] gravy;
	delete[] gravx;
	delete[] gravp;
	delete[] gravmask;
}

void Gravity::Clear()
{
	int size = (XRES / CELL) * (YRES / CELL);
	std::fill(gravy, gravy + size, 0.0f);
	std::fill(gravx, gravx + size, 0.0f);
	std::fill(gravp, gravp + size, 0.0f);
	std::fill(gravmap, gravmap + size, 0.0f);
	std::fill(gravmask, gravmask + size, 0xFFFFFFFF);

	ignoreNextResult = true;
}

#ifdef GRAVFFT
void Gravity::grav_fft_init()
{
	int xblock2 = XRES/CELL*2;
	int yblock2 = YRES/CELL*2;
	int fft_tsize = (xblock2/2+1)*yblock2;
	float distance, scaleFactor;
	fftwf_plan plan_ptgravx, plan_ptgravy;
	if (grav_fft_status) return;

	//use fftw malloc function to ensure arrays are aligned, to get better performance
	th_ptgravx = reinterpret_cast<float*>(fftwf_malloc(xblock2 * yblock2 * sizeof(float)));
	th_ptgravy = reinterpret_cast<float*>(fftwf_malloc(xblock2 * yblock2 * sizeof(float)));
	th_ptgravxt = reinterpret_cast<fftwf_complex*>(fftwf_malloc(fft_tsize * sizeof(fftwf_complex)));
	th_ptgravyt = reinterpret_cast<fftwf_complex*>(fftwf_malloc(fft_tsize * sizeof(fftwf_complex)));
	th_gravmapbig = reinterpret_cast<float*>(fftwf_malloc(xblock2 * yblock2 * sizeof(float)));
	th_gravmapbigt = reinterpret_cast<fftwf_complex*>(fftwf_malloc(fft_tsize * sizeof(fftwf_complex)));
	th_gravxbig = reinterpret_cast<float*>(fftwf_malloc(xblock2 * yblock2 * sizeof(float)));
	th_gravybig = reinterpret_cast<float*>(fftwf_malloc(xblock2 * yblock2 * sizeof(float)));
	th_gravxbigt = reinterpret_cast<fftwf_complex*>(fftwf_malloc(fft_tsize * sizeof(fftwf_complex)));
	th_gravybigt = reinterpret_cast<fftwf_complex*>(fftwf_malloc(fft_tsize * sizeof(fftwf_complex)));

	//select best algorithm, could use FFTW_PATIENT or FFTW_EXHAUSTIVE but that increases the time taken to plan, and I don't see much increase in execution speed
	plan_ptgravx = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravx, th_ptgravxt, FFTW_MEASURE);
	plan_ptgravy = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravy, th_ptgravyt, FFTW_MEASURE);
	plan_gravmap = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_gravmapbig, th_gravmapbigt, FFTW_MEASURE);
	plan_gravx_inverse = fftwf_plan_dft_c2r_2d(yblock2, xblock2, th_gravxbigt, th_gravxbig, FFTW_MEASURE);
	plan_gravy_inverse = fftwf_plan_dft_c2r_2d(yblock2, xblock2, th_gravybigt, th_gravybig, FFTW_MEASURE);

	//(XRES/CELL)*(YRES/CELL)*4 is size of data array, scaling needed because FFTW calculates an unnormalized DFT
	scaleFactor = -M_GRAV/((XRES/CELL)*(YRES/CELL)*4);
	//calculate velocity map caused by a point mass
	for (int y = 0; y < yblock2; y++)
	{
		for (int x = 0; x < xblock2; x++)
		{
			if (x == XRES / CELL && y == YRES / CELL)
				continue;
			distance = sqrtf(pow(x-(XRES/CELL), 2.0f) + pow(y-(YRES/CELL), 2.0f));
			th_ptgravx[y * xblock2 + x] = scaleFactor * (x - (XRES / CELL)) / pow(distance, 3);
			th_ptgravy[y * xblock2 + x] = scaleFactor * (y - (YRES / CELL)) / pow(distance, 3);
		}
	}
	th_ptgravx[yblock2 * xblock2 / 2 + xblock2 / 2] = 0.0f;
	th_ptgravy[yblock2 * xblock2 / 2 + xblock2 / 2] = 0.0f;

	//transform point mass velocity maps
	fftwf_execute(plan_ptgravx);
	fftwf_execute(plan_ptgravy);
	fftwf_destroy_plan(plan_ptgravx);
	fftwf_destroy_plan(plan_ptgravy);
	fftwf_free(th_ptgravx);
	fftwf_free(th_ptgravy);

	//clear padded gravmap
	memset(th_gravmapbig, 0, xblock2 * yblock2 * sizeof(float));

	grav_fft_status = true;
}

void Gravity::grav_fft_cleanup()
{
	if (!grav_fft_status) return;
	fftwf_free(th_ptgravxt);
	fftwf_free(th_ptgravyt);
	fftwf_free(th_gravmapbig);
	fftwf_free(th_gravmapbigt);
	fftwf_free(th_gravxbig);
	fftwf_free(th_gravybig);
	fftwf_free(th_gravxbigt);
	fftwf_free(th_gravybigt);
	fftwf_destroy_plan(plan_gravmap);
	fftwf_destroy_plan(plan_gravx_inverse);
	fftwf_destroy_plan(plan_gravy_inverse);
	grav_fft_status = false;
}
#endif

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
#if !defined(GRAVFFT) && defined(GRAV_DIFF)
					memcpy(gravy, th_gravy, (XRES/CELL)*(YRES/CELL)*sizeof(float));
					memcpy(gravx, th_gravx, (XRES/CELL)*(YRES/CELL)*sizeof(float));
					memcpy(gravp, th_gravp, (XRES/CELL)*(YRES/CELL)*sizeof(float));
#else
					// Copy thread gravity maps into this one
					std::swap(gravy, th_gravy);
					std::swap(gravx, th_gravx);
					std::swap(gravp, th_gravp);
#endif
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
	unsigned int size = (XRES / CELL) * (YRES / CELL);
	membwand(gravy, gravmask, size * sizeof(float), size * sizeof(unsigned));
	membwand(gravx, gravmask, size * sizeof(float), size * sizeof(unsigned));
	std::fill(&gravmap[0], &gravmap[size], 0.0f);
}

void Gravity::update_grav_async()
{
	int done = 0;
	int thread_done = 0;
	unsigned int size = (XRES / CELL) * (YRES / CELL);
	std::fill(&th_ogravmap[0], &th_ogravmap[size], 0.0f);
	std::fill(&th_gravmap[0], &th_gravmap[size], 0.0f);
	std::fill(&th_gravy[0], &th_gravy[size], 0.0f);
	std::fill(&th_gravx[0], &th_gravx[size], 0.0f);
	std::fill(&th_gravp[0], &th_gravp[size], 0.0f);

#ifdef GRAVFFT
	if (!grav_fft_status)
		grav_fft_init();
#endif

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

	unsigned int size = (XRES / CELL) * (YRES / CELL);
	std::fill(&gravy[0], &gravy[size], 0.0f);
	std::fill(&gravx[0], &gravx[size], 0.0f);
	std::fill(&gravp[0], &gravp[size], 0.0f);
	std::fill(&gravmap[0], &gravmap[size], 0.0f);
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
	unsigned int size = (XRES / CELL) * (YRES / CELL);
	std::fill(&gravy[0], &gravy[size], 0.0f);
	std::fill(&gravx[0], &gravx[size], 0.0f);
	std::fill(&gravp[0], &gravp[size], 0.0f);
	std::fill(&gravmap[0], &gravmap[size], 0.0f);
}

#ifdef GRAVFFT
void Gravity::update_grav()
{
	int xblock2 = XRES/CELL*2, yblock2 = YRES/CELL*2;
	int fft_tsize = (xblock2/2+1)*yblock2;
	float mr, mc, pr, pc, gr, gc;
	if (memcmp(th_ogravmap, th_gravmap, sizeof(float)*(XRES/CELL)*(YRES/CELL)) != 0)
	{
		th_gravchanged = 1;

		membwand(th_gravmap, gravmask, (XRES/CELL)*(YRES/CELL)*sizeof(float), (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
		//copy gravmap into padded gravmap array
		for (int y = 0; y < YRES / CELL; y++)
		{
			for (int x = 0; x < XRES / CELL; x++)
			{
				th_gravmapbig[(y+YRES/CELL)*xblock2+XRES/CELL+x] = th_gravmap[y*(XRES/CELL)+x];
			}
		}
		//transform gravmap
		fftwf_execute(plan_gravmap);
		//do convolution (multiply the complex numbers)
		for (int i = 0; i < fft_tsize; i++)
		{
			mr = th_gravmapbigt[i][0];
			mc = th_gravmapbigt[i][1];
			pr = th_ptgravxt[i][0];
			pc = th_ptgravxt[i][1];
			gr = mr*pr-mc*pc;
			gc = mr*pc+mc*pr;
			th_gravxbigt[i][0] = gr;
			th_gravxbigt[i][1] = gc;
			pr = th_ptgravyt[i][0];
			pc = th_ptgravyt[i][1];
			gr = mr*pr-mc*pc;
			gc = mr*pc+mc*pr;
			th_gravybigt[i][0] = gr;
			th_gravybigt[i][1] = gc;
		}
		//inverse transform, and copy from padded arrays into normal velocity maps
		fftwf_execute(plan_gravx_inverse);
		fftwf_execute(plan_gravy_inverse);
		for (int y = 0; y < YRES / CELL; y++)
		{
			for (int x = 0; x < XRES / CELL; x++)
			{
				th_gravx[y*(XRES/CELL)+x] = th_gravxbig[y*xblock2+x];
				th_gravy[y*(XRES/CELL)+x] = th_gravybig[y*xblock2+x];
				th_gravp[y*(XRES/CELL)+x] = sqrtf(pow(th_gravxbig[y*xblock2+x],2)+pow(th_gravybig[y*xblock2+x],2));
			}
		}
	}
	else
	{
		th_gravchanged = 0;
	}

	// Copy th_ogravmap into th_gravmap (doesn't matter what th_ogravmap is afterwards)
	std::swap(th_gravmap, th_ogravmap);
}

#else
// gravity without fast Fourier transforms

void Gravity::update_grav(void)
{
	int x, y, i, j;
	float val, distance;
	th_gravchanged = 0;
#ifndef GRAV_DIFF
	//Find any changed cells
	for (i=0; i<YRES/CELL; i++)
	{
		if(changed)
			break;
		for (j=0; j<XRES/CELL; j++)
		{
			if(th_ogravmap[i*(XRES/CELL)+j]!=th_gravmap[i*(XRES/CELL)+j]){
				changed = 1;
				break;
			}
		}
	}
	if(!changed)
		goto fin;
	memset(th_gravy, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(th_gravx, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
#endif
	th_gravchanged = 1;
	membwand(th_gravmap, gravmask, (XRES/CELL)*(YRES/CELL)*sizeof(float), (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
	for (i = 0; i < YRES / CELL; i++) {
		for (j = 0; j < XRES / CELL; j++) {
#ifdef GRAV_DIFF
			if (th_ogravmap[i*(XRES/CELL)+j] != th_gravmap[i*(XRES/CELL)+j])
			{
#else
			if (th_gravmap[i*(XRES/CELL)+j] > 0.0001f || th_gravmap[i*(XRES/CELL)+j]<-0.0001f) //Only calculate with populated or changed cells.
			{
#endif
				for (y = 0; y < YRES / CELL; y++) {
					for (x = 0; x < XRES / CELL; x++) {
						if (x == j && y == i)//Ensure it doesn't calculate with itself
							continue;
						distance = sqrt(pow(j - x, 2.0f) + pow(i - y, 2.0f));
#ifdef GRAV_DIFF
						val = th_gravmap[i*(XRES/CELL)+j] - th_ogravmap[i*(XRES/CELL)+j];
#else
						val = th_gravmap[i*(XRES/CELL)+j];
#endif
						th_gravx[y*(XRES/CELL)+x] += M_GRAV * val * (j - x) / pow(distance, 3.0f);
						th_gravy[y*(XRES/CELL)+x] += M_GRAV * val * (i - y) / pow(distance, 3.0f);
						th_gravp[y*(XRES/CELL)+x] += M_GRAV * val / pow(distance, 2.0f);
					}
				}
			}
		}
	}
	memcpy(th_ogravmap, th_gravmap, (XRES/CELL)*(YRES/CELL)*sizeof(float));
}
#endif



bool Gravity::grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL])
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
			while (x2 <= XRES/CELL-1)
			{
				if (x2 == XRES/CELL-1)
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
			if (y < YRES/CELL-1)
				for (x=x1; x<=x2; x++)
					if (!checkmap[y+1][x] && bmap[y+1][x] != WL_GRAV)
					{
						if (y+1 == YRES/CELL-1)
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
	char checkmap[YRES/CELL][XRES/CELL];
	unsigned maskvalue;
	mask_el *t_mask_el = nullptr;
	mask_el *c_mask_el = nullptr;
	if (!gravmask)
		return;
	memset(checkmap, 0, sizeof(checkmap));
	for (int x = 0; x < XRES / CELL; x++)
	{
		for(int y = 0; y < YRES / CELL; y++)
		{
			if (bmap[y][x] != WL_GRAV && checkmap[y][x] == 0)
			{
				// Create a new shape
				if (t_mask_el == nullptr)
				{
					t_mask_el = new mask_el[sizeof(mask_el)];
					t_mask_el->shape = new char[(XRES / CELL) * (YRES / CELL)];
					std::fill(&t_mask_el->shape[0], &t_mask_el->shape[(XRES / CELL) * (YRES / CELL)], 0);
					t_mask_el->shapeout = 0;
					t_mask_el->next = nullptr;
					c_mask_el = t_mask_el;
				}
				else
				{
					c_mask_el->next = new mask_el[sizeof(mask_el)];
					c_mask_el = c_mask_el->next;
					c_mask_el->shape = new char[(XRES / CELL) * (YRES / CELL)];
					std::fill(&c_mask_el->shape[0], &c_mask_el->shape[(XRES / CELL) * (YRES / CELL)], 0);
					c_mask_el->shapeout = 0;
					c_mask_el->next = nullptr;
				}
				// Fill the shape
				if (grav_mask_r(x, y, checkmap, reinterpret_cast<char(*)[XRES/CELL]>(c_mask_el->shape)))
					c_mask_el->shapeout = 1;
			}
		}
	}
	c_mask_el = t_mask_el;
	std::fill(&gravmask[0], &gravmask[(XRES / CELL) * (YRES / CELL)], 0);
	while (c_mask_el != nullptr)
	{
		char *cshape = c_mask_el->shape;
		for (int x = 0; x < XRES / CELL; x++)
		{
			for (int y = 0; y < YRES / CELL; y++)
			{
				if (cshape[y * (XRES / CELL) + x])
				{
					if (c_mask_el->shapeout)
						maskvalue = 0xFFFFFFFF;
					else
						maskvalue = 0x00000000;
					gravmask[y * (XRES / CELL) + x] = maskvalue;
				}
			}
		}
		c_mask_el = c_mask_el->next;
	}
	mask_free(t_mask_el);
}
