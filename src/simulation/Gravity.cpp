#include <cmath>
#include <sys/types.h>
#include "common/tpt-thread.h"
#include "Config.h"
#include "Gravity.h"
#include "Misc.h"

void Gravity::bilinear_interpolation(float *src, float *dst, int sw, int sh, int rw, int rh)
{
	int y, x, fxceil, fyceil;
	float fx, fy, fyc, fxc;
	double intp;
	float tr, tl, br, bl;
	//Bilinear interpolation for upscaling
	for (y=0; y<rh; y++)
		for (x=0; x<rw; x++)
		{
			fx = ((float)x)*((float)sw)/((float)rw);
			fy = ((float)y)*((float)sh)/((float)rh);
			fxc = modf(fx, &intp);
			fyc = modf(fy, &intp);
			fxceil = (int)ceil(fx);
			fyceil = (int)ceil(fy);
			if (fxceil>=sw) fxceil = sw-1;
			if (fyceil>=sh) fyceil = sh-1;
			tr = src[sw*(int)floor(fy)+fxceil];
			tl = src[sw*(int)floor(fy)+(int)floor(fx)];
			br = src[sw*fyceil+fxceil];
			bl = src[sw*fyceil+(int)floor(fx)];
			dst[rw*y+x] = ((tl*(1.0f-fxc))+(tr*(fxc)))*(1.0f-fyc) + ((bl*(1.0f-fxc))+(br*(fxc)))*(fyc);				
		}
}

void Gravity::Clear()
{
	std::fill(gravy, gravy+((XRES/CELL)*(YRES/CELL)), 0.0f);
	std::fill(gravx, gravx+((XRES/CELL)*(YRES/CELL)), 0.0f);
	std::fill(gravp, gravp+((XRES/CELL)*(YRES/CELL)), 0.0f);
	std::fill(gravmap, gravmap+((XRES/CELL)*(YRES/CELL)), 0.0f);
	std::fill(gravmask, gravmask+((XRES/CELL)*(YRES/CELL)), 0xFFFFFFFF);
}

void Gravity::gravity_init()
{
	ngrav_enable = 0;
	//Allocate full size Gravmaps
	th_ogravmap = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	th_gravmap = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	th_gravy = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	th_gravx = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	th_gravp = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	gravmap = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	gravy = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	gravx = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	gravp = (float *)calloc((XRES/CELL)*(YRES/CELL), sizeof(float));
	gravmask = (unsigned int *)calloc((XRES/CELL)*(YRES/CELL), sizeof(unsigned));
	obmap = (unsigned char (*)[XRES/CELL])calloc((XRES/CELL)*(YRES/CELL), sizeof(unsigned char));
}

void Gravity::gravity_cleanup()
{
	stop_grav_async();
#ifdef GRAVFFT
	grav_fft_cleanup();
#endif
	//Free gravity info
	free(th_ogravmap);
	free(th_gravmap);
	free(th_gravy);
	free(th_gravx);
	free(th_gravp);
	free(gravmap);
	free(gravy);
	free(gravx);
	free(gravp);
	free(gravmask);
	free(obmap);
}

void Gravity::gravity_update_async()
{
	int result;
	if(ngrav_enable)
	{
		pthread_mutex_lock(&gravmutex);
		result = grav_ready;
		if(result) //Did the gravity thread finish?
		{
			//if (!sys_pause||framerender){ //Only update if not paused
				//Switch the full size gravmaps, we don't really need the two above any more
				float *tmpf;

				if(th_gravchanged)
				{
				#if !defined(GRAVFFT) && defined(GRAV_DIFF)
					memcpy(gravy, th_gravy, (XRES/CELL)*(YRES/CELL)*sizeof(float));
					memcpy(gravx, th_gravx, (XRES/CELL)*(YRES/CELL)*sizeof(float));
					memcpy(gravp, th_gravp, (XRES/CELL)*(YRES/CELL)*sizeof(float));
				#else
					tmpf = gravy;
					gravy = th_gravy;
					th_gravy = tmpf;

					tmpf = gravx;
					gravx = th_gravx;
					th_gravx = tmpf;

					tmpf = gravp;
					gravp = th_gravp;
					th_gravp = tmpf;
				#endif
				}

				tmpf = gravmap;
				gravmap = th_gravmap;
				th_gravmap = tmpf;

				grav_ready = 0; //Tell the other thread that we're ready for it to continue
				pthread_cond_signal(&gravcv);
			//}
		}
		pthread_mutex_unlock(&gravmutex);
		//Apply the gravity mask
		membwand(gravy, gravmask, (XRES/CELL)*(YRES/CELL)*sizeof(float), (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
		membwand(gravx, gravmask, (XRES/CELL)*(YRES/CELL)*sizeof(float), (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
		memset(gravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	}
}

TH_ENTRY_POINT void *Gravity::update_grav_async_helper(void * context)
{
	((Gravity *)context)->update_grav_async();
	return NULL;
}

void Gravity::update_grav_async()
{
	int done = 0;
	int thread_done = 0;
	memset(th_ogravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(th_gravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(th_gravy, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(th_gravx, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(th_gravp, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	//memset(th_gravy, 0, XRES*YRES*sizeof(float));
	//memset(th_gravx, 0, XRES*YRES*sizeof(float));
	//memset(th_gravp, 0, XRES*YRES*sizeof(float));
#ifdef GRAVFFT
	if (!grav_fft_status)
		grav_fft_init();
#endif
	while(!thread_done){
		if(!done){
			update_grav();
			done = 1;
			pthread_mutex_lock(&gravmutex);
			
			grav_ready = done;
			thread_done = gravthread_done;
			
			pthread_mutex_unlock(&gravmutex);
		} else {
			pthread_mutex_lock(&gravmutex);
			pthread_cond_wait(&gravcv, &gravmutex);
		    
			done = grav_ready;
			thread_done = gravthread_done;
			
			pthread_mutex_unlock(&gravmutex);
		}
	}
	pthread_exit(NULL);
}

void Gravity::start_grav_async()
{
	if(ngrav_enable)	//If it's already enabled, restart it
		stop_grav_async();

	gravthread_done = 0;
	grav_ready = 0;
	pthread_mutex_init (&gravmutex, NULL);
	pthread_cond_init(&gravcv, NULL);
	pthread_create(&gravthread, NULL, &Gravity::update_grav_async_helper, this); //Start asynchronous gravity simulation
	ngrav_enable = 1;

	memset(gravy, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravx, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravp, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
}

void Gravity::stop_grav_async()
{
	if(ngrav_enable){
		pthread_mutex_lock(&gravmutex);
		gravthread_done = 1;
		pthread_cond_signal(&gravcv);
		pthread_mutex_unlock(&gravmutex);
		pthread_join(gravthread, NULL);
		pthread_mutex_destroy(&gravmutex); //Destroy the mutex
		ngrav_enable = 0;
	}
	//Clear the grav velocities
	memset(gravy, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravx, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravp, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memset(gravmap, 0, (XRES/CELL)*(YRES/CELL)*sizeof(float));
}

#ifdef GRAVFFT

void Gravity::grav_fft_init()
{
	int xblock2 = XRES/CELL*2;
	int yblock2 = YRES/CELL*2;
	int x, y, fft_tsize = (xblock2/2+1)*yblock2;
	float distance, scaleFactor;
	fftwf_plan plan_ptgravx, plan_ptgravy;
	if (grav_fft_status) return;

	//use fftw malloc function to ensure arrays are aligned, to get better performance
	th_ptgravx = (float*)fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_ptgravy = (float*)fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_ptgravxt = (fftwf_complex*)fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_ptgravyt = (fftwf_complex*)fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravmapbig = (float*)fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravmapbigt = (fftwf_complex*)fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravxbig = (float*)fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravybig = (float*)fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravxbigt = (fftwf_complex*)fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravybigt = (fftwf_complex*)fftwf_malloc(fft_tsize*sizeof(fftwf_complex));

	//select best algorithm, could use FFTW_PATIENT or FFTW_EXHAUSTIVE but that increases the time taken to plan, and I don't see much increase in execution speed
	plan_ptgravx = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravx, th_ptgravxt, FFTW_MEASURE);
	plan_ptgravy = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravy, th_ptgravyt, FFTW_MEASURE);
	plan_gravmap = fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_gravmapbig, th_gravmapbigt, FFTW_MEASURE);
	plan_gravx_inverse = fftwf_plan_dft_c2r_2d(yblock2, xblock2, th_gravxbigt, th_gravxbig, FFTW_MEASURE);
	plan_gravy_inverse = fftwf_plan_dft_c2r_2d(yblock2, xblock2, th_gravybigt, th_gravybig, FFTW_MEASURE);

	//(XRES/CELL)*(YRES/CELL)*4 is size of data array, scaling needed because FFTW calculates an unnormalized DFT
	scaleFactor = -M_GRAV/((XRES/CELL)*(YRES/CELL)*4);
	//calculate velocity map caused by a point mass
	for (y=0; y<yblock2; y++)
	{
		for (x=0; x<xblock2; x++)
		{
			if (x==XRES/CELL && y==YRES/CELL) continue;
			distance = sqrtf(pow(x-(XRES/CELL), 2.0f) + pow(y-(YRES/CELL), 2.0f));
			th_ptgravx[y*xblock2+x] = scaleFactor*(x-(XRES/CELL)) / pow(distance, 3.0f);
			th_ptgravy[y*xblock2+x] = scaleFactor*(y-(YRES/CELL)) / pow(distance, 3.0f);
		}
	}
	th_ptgravx[yblock2*xblock2/2+xblock2/2] = 0.0f;
	th_ptgravy[yblock2*xblock2/2+xblock2/2] = 0.0f;

	//transform point mass velocity maps
	fftwf_execute(plan_ptgravx);
	fftwf_execute(plan_ptgravy);
	fftwf_destroy_plan(plan_ptgravx);
	fftwf_destroy_plan(plan_ptgravy);
	fftwf_free(th_ptgravx);
	fftwf_free(th_ptgravy);

	//clear padded gravmap
	memset(th_gravmapbig,0,xblock2*yblock2*sizeof(float));

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

void Gravity::update_grav()
{
	int x, y, changed = 0;
	int xblock2 = XRES/CELL*2, yblock2 = YRES/CELL*2;
	int i, fft_tsize = (xblock2/2+1)*yblock2;
	float mr, mc, pr, pc, gr, gc;
	for (y=0; y<YRES/CELL; y++)
	{
		if(changed)
			break;
		for (x=0; x<XRES/CELL; x++)
		{
			if(th_ogravmap[y*(XRES/CELL)+x] != th_gravmap[y*(XRES/CELL)+x] || bmap[y][x] != obmap[y][x]){
				changed = 1;
				break;
			}
		}
	}
	if(changed)
	{
		th_gravchanged = 1;

		membwand(th_gravmap, gravmask, (XRES/CELL)*(YRES/CELL)*sizeof(float), (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
		//copy gravmap into padded gravmap array
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
			{
				th_gravmapbig[(y+YRES/CELL)*xblock2+XRES/CELL+x] = th_gravmap[y*(XRES/CELL)+x];
			}
		}
		//transform gravmap
		fftwf_execute(plan_gravmap);
		//do convolution (multiply the complex numbers)
		for (i=0; i<fft_tsize; i++)
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
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
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
	memcpy(th_ogravmap, th_gravmap, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memcpy(obmap, bmap, (XRES/CELL)*(YRES/CELL)*sizeof(unsigned char));
}

#else
// gravity without fast Fourier transforms

void Gravity::update_grav(void)
{
	int x, y, i, j, changed = 0;
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
fin:
	memcpy(th_ogravmap, th_gravmap, (XRES/CELL)*(YRES/CELL)*sizeof(float));
	memcpy(obmap, bmap, (XRES/CELL)*(YRES/CELL)*sizeof(unsigned char));
}
#endif



void Gravity::grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL], char *shapeout)
{
	if(x < 0 || x >= XRES/CELL || y < 0 || y >= YRES/CELL)
		return;
	if(x == 0 || y ==0 || y == (YRES/CELL)-1 || x == (XRES/CELL)-1)
		*shapeout = 1;

	int x1 = x, x2 = x;
	while (x1 >= 1)
	{
		if(checkmap[y][x1-1] || bmap[y][x1-1]==WL_GRAV)
			break;
		x1--;
	}
	while (x2 < (XRES/CELL)-1)
	{
		if(checkmap[y][x2+1] || bmap[y][x2+1]==WL_GRAV)
			break;
		x2++;
	}
	
	// fill span
	for (x = x1; x <= x2; x++)
		checkmap[y][x] = shape[y][x] = 1;

	if(y >= 1)
		for(x = x1; x <= x2; x++)
			if(!checkmap[y-1][x] && bmap[y-1][x]!=WL_GRAV)
				grav_mask_r(x, y-1, checkmap, shape, shapeout);
	if(y < (YRES/CELL)-1)
		for(x = x1; x <= x2; x++)
			if(!checkmap[y+1][x] && bmap[y+1][x]!=WL_GRAV)
				grav_mask_r(x, y+1, checkmap, shape, shapeout);
	return;
}
void Gravity::mask_free(mask_el *c_mask_el){
	if(c_mask_el==NULL)
		return;
	if(c_mask_el->next!=NULL)
		mask_free((mask_el*)c_mask_el->next);
	free(c_mask_el->shape);
	free(c_mask_el);
}
void Gravity::gravity_mask()
{
	char checkmap[YRES/CELL][XRES/CELL];
	int x = 0, y = 0;
	unsigned maskvalue;
	mask_el *t_mask_el = NULL;
	mask_el *c_mask_el = NULL;
	if(!gravmask)
		return;
	memset(checkmap, 0, sizeof(checkmap));
	for(x = 0; x < XRES/CELL; x++)
	{
		for(y = 0; y < YRES/CELL; y++)
		{
			if(bmap[y][x]!=WL_GRAV && checkmap[y][x] == 0)
			{
				//Create a new shape
				if(t_mask_el==NULL){
					t_mask_el = (mask_el *)malloc(sizeof(mask_el));
					t_mask_el->shape = (char *)malloc((XRES/CELL)*(YRES/CELL));
					memset(t_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					t_mask_el->shapeout = 0;
					t_mask_el->next = NULL;
					c_mask_el = t_mask_el;
				} else {
					c_mask_el->next = (mask_el *)malloc(sizeof(mask_el));
					c_mask_el = (mask_el *)c_mask_el->next;
					c_mask_el->shape = (char *)malloc((XRES/CELL)*(YRES/CELL));
					memset(c_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					c_mask_el->shapeout = 0;
					c_mask_el->next = NULL;
				}
				//Fill the shape
				grav_mask_r(x, y, (char (*)[XRES/CELL])checkmap, (char (*)[XRES/CELL])c_mask_el->shape, (char*)&c_mask_el->shapeout);
			}
		}
	}
	c_mask_el = t_mask_el;
	memset(gravmask, 0, (XRES/CELL)*(YRES/CELL)*sizeof(unsigned));
	while(c_mask_el!=NULL)
	{
		char *cshape = c_mask_el->shape;
		for(x = 0; x < XRES/CELL; x++)
		{
			for(y = 0; y < YRES/CELL; y++)
			{
				if(cshape[y*(XRES/CELL)+x]){
					if(c_mask_el->shapeout)
						maskvalue = 0xFFFFFFFF;
					else
						maskvalue = 0x00000000;
					gravmask[y*(XRES/CELL)+x] = maskvalue;
				}
			}
		}
		c_mask_el = (mask_el*)c_mask_el->next;
	}
	mask_free(t_mask_el);
}
#ifdef GRAVFFT
Gravity::Gravity():
	grav_fft_status(false)
{
	gravity_init();
}
#else
Gravity::Gravity()
{
	gravity_init();
}
#endif
Gravity::~Gravity()
{
	gravity_cleanup();
}
