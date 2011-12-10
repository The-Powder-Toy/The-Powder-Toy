#include <math.h>
#include <pthread.h>
#include "defines.h"
#include "gravity.h"
#include "powder.h"

#ifdef GRAVFFT
#include <fftw3.h>
#endif 


float gravmap[YRES/CELL][XRES/CELL];  //Maps to be used by the main thread
float *gravpf;
float *gravyf;
float *gravxf;
unsigned gravmask[YRES/CELL][XRES/CELL];

float th_ogravmap[YRES/CELL][XRES/CELL]; // Maps to be processed by the gravity thread
float th_gravmap[YRES/CELL][XRES/CELL];
float th_gravx[YRES/CELL][XRES/CELL];
float th_gravy[YRES/CELL][XRES/CELL];
float th_gravp[YRES/CELL][XRES/CELL];
float *th_gravpf;
float *th_gravyf;
float *th_gravxf;

int gravwl_timeout = 0;
int gravityMode = 0; // starts enabled in "vertical" mode...
int ngrav_enable = 0; //Newtonian gravity, will be set by save

pthread_t gravthread;
pthread_mutex_t gravmutex;
pthread_cond_t gravcv;
int grav_ready = 0;
int gravthread_done = 0;

void bilinear_interpolation(float *src, float *dst, int sw, int sh, int rw, int rh)
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

void gravity_init()
{
	//Allocate full size Gravmaps
	th_gravyf = calloc(XRES*YRES, sizeof(float));
	th_gravxf = calloc(XRES*YRES, sizeof(float));
	th_gravpf = calloc(XRES*YRES, sizeof(float));
	gravyf = calloc(XRES*YRES, sizeof(float));
	gravxf = calloc(XRES*YRES, sizeof(float));
	gravpf = calloc(XRES*YRES, sizeof(float));
}

void gravity_cleanup()
{
#ifdef GRAVFFT
	grav_fft_cleanup();
#endif
}

void gravity_update_async()
{
	int result;
	if(ngrav_enable)
	{
		pthread_mutex_lock(&gravmutex);
		result = grav_ready;
		if(result) //Did the gravity thread finish?
		{
			memcpy(th_gravmap, gravmap, sizeof(gravmap)); //Move our current gravmap to be processed other thread

			if (!sys_pause||framerender){ //Only update if not paused
				//Switch the full size gravmaps, we don't really need the two above any more
				float *tmpf;
				tmpf = gravyf;
				gravyf = th_gravyf;
				th_gravyf = tmpf;

				tmpf = gravxf;
				gravxf = th_gravxf;
				th_gravxf = tmpf;

				tmpf = gravpf;
				gravpf = th_gravpf;
				th_gravpf = tmpf;

				grav_ready = 0; //Tell the other thread that we're ready for it to continue
				pthread_cond_signal(&gravcv);
			}
		}
		pthread_mutex_unlock(&gravmutex);
		//Apply the gravity mask
		//TODO: doesn't work at the moment, gravx and gravy aren't used any more
		//membwand(gravy, gravmask, sizeof(gravy), sizeof(gravmask));
		//membwand(gravx, gravmask, sizeof(gravx), sizeof(gravmask));
	}
}

void* update_grav_async(void* unused)
{
	int done = 0;
	int thread_done = 0;
	memset(th_ogravmap, 0, sizeof(th_ogravmap));
	memset(th_gravmap, 0, sizeof(th_gravmap));
	memset(th_gravy, 0, sizeof(th_gravy));
	memset(th_gravx, 0, sizeof(th_gravx));
	memset(th_gravyf, 0, XRES*YRES*sizeof(float));
	memset(th_gravxf, 0, XRES*YRES*sizeof(float));
	memset(th_gravpf, 0, XRES*YRES*sizeof(float));
#ifdef GRAVFFT
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

void start_grav_async()
{
	if(!ngrav_enable){
		gravthread_done = 0;
		grav_ready = 0;
		pthread_mutex_init (&gravmutex, NULL);
		pthread_cond_init(&gravcv, NULL);
		pthread_create(&gravthread, NULL, update_grav_async, NULL); //Start asynchronous gravity simulation
		ngrav_enable = 1;
	}
	memset(gravyf, 0, XRES*YRES*sizeof(float));
	memset(gravxf, 0, XRES*YRES*sizeof(float));
	memset(gravpf, 0, XRES*YRES*sizeof(float));
}

void stop_grav_async()
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
	memset(gravyf, 0, XRES*YRES*sizeof(float));
	memset(gravxf, 0, XRES*YRES*sizeof(float));
	memset(gravpf, 0, XRES*YRES*sizeof(float));
}

#ifdef GRAVFFT
int grav_fft_status = 0;
float *th_ptgravx, *th_ptgravy, *th_gravmapbig, *th_gravxbig, *th_gravybig;
fftwf_complex *th_ptgravxt, *th_ptgravyt, *th_gravmapbigt, *th_gravxbigt, *th_gravybigt;
fftwf_plan plan_gravmap, plan_gravx_inverse, plan_gravy_inverse;

void grav_fft_init()
{
	int xblock2 = XRES/CELL*2;
	int yblock2 = YRES/CELL*2;
	int x, y, fft_tsize = (xblock2/2+1)*yblock2;
	float distance, scaleFactor;
	fftwf_plan plan_ptgravx, plan_ptgravy;
	if (grav_fft_status) return;

	//use fftw malloc function to ensure arrays are aligned, to get better performance
	th_ptgravx = fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_ptgravy = fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_ptgravxt = fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_ptgravyt = fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravmapbig = fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravmapbigt = fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravxbig = fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravybig = fftwf_malloc(xblock2*yblock2*sizeof(float));
	th_gravxbigt = fftwf_malloc(fft_tsize*sizeof(fftwf_complex));
	th_gravybigt = fftwf_malloc(fft_tsize*sizeof(fftwf_complex));

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
			distance = sqrtf(pow(x-(XRES/CELL), 2) + pow(y-(YRES/CELL), 2));
			th_ptgravx[y*xblock2+x] = scaleFactor*(x-(XRES/CELL)) / pow(distance, 3);
			th_ptgravy[y*xblock2+x] = scaleFactor*(y-(YRES/CELL)) / pow(distance, 3);
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

	grav_fft_status = 1;
}

void grav_fft_cleanup()
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
	grav_fft_status = 0;
}

void update_grav()
{
	int x, y, changed = 0;
	for (y=0; y<YRES/CELL; y++)
	{
		if(changed)
			break;
		for (x=0; x<XRES/CELL; x++)
		{
			if(th_ogravmap[y][x]!=th_gravmap[y][x]){
				changed = 1;
				break;
			}
		}
	}
	if(changed)
	{
		int xblock2 = XRES/CELL*2, yblock2 = YRES/CELL*2;
		int i, fft_tsize = (xblock2/2+1)*yblock2;
		float mr, mc, pr, pc, gr, gc;
		if (!grav_fft_status) grav_fft_init();

		//copy gravmap into padded gravmap array
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
			{
				th_gravmapbig[(y+YRES/CELL)*xblock2+XRES/CELL+x] = th_gravmap[y][x];
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
				th_gravx[y][x] = th_gravxbig[y*xblock2+x];
				th_gravy[y][x] = th_gravybig[y*xblock2+x];
				th_gravp[y][x] = sqrtf(pow(th_gravxbig[y*xblock2+x],2)+pow(th_gravybig[y*xblock2+x],2));
			}
		}
	}
	memcpy(th_ogravmap, th_gravmap, sizeof(th_gravmap));
	bilinear_interpolation(th_gravy, th_gravyf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravx, th_gravxf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravp, th_gravpf, XRES/CELL, YRES/CELL, XRES, YRES);
}

#else
// gravity without fast Fourier transforms

void update_grav(void)
{
	int x, y, i, j, changed = 0;
	float val, distance;
#ifndef GRAV_DIFF
	//Find any changed cells
	for (i=0; i<YRES/CELL; i++)
	{
		if(changed)
			break;
		for (j=0; j<XRES/CELL; j++)
		{
			if(th_ogravmap[i][j]!=th_gravmap[i][j]){
				changed = 1;
				break;
			}
		}
	}
	if(!changed)
		goto fin;
	memset(th_gravy, 0, sizeof(th_gravy));
	memset(th_gravx, 0, sizeof(th_gravx));
#endif
	for (i = 0; i < YRES / CELL; i++) {
		for (j = 0; j < XRES / CELL; j++) {
#ifdef GRAV_DIFF
			if (th_ogravmap[i][j] != th_gravmap[i][j])
			{
#else
			if (th_gravmap[i][j] > 0.0001f || th_gravmap[i][j]<-0.0001f) //Only calculate with populated or changed cells.
			{
#endif
				for (y = 0; y < YRES / CELL; y++) {
					for (x = 0; x < XRES / CELL; x++) {
						if (x == j && y == i)//Ensure it doesn't calculate with itself
							continue;
						distance = sqrt(pow(j - x, 2) + pow(i - y, 2));
#ifdef GRAV_DIFF
						val = th_gravmap[i][j] - th_ogravmap[i][j];
#else
						val = th_gravmap[i][j];
#endif
						th_gravx[y][x] += M_GRAV * val * (j - x) / pow(distance, 3);
						th_gravy[y][x] += M_GRAV * val * (i - y) / pow(distance, 3);
						th_gravp[y][x] += M_GRAV * val / pow(distance, 2);
					}
				}
			}
		}
	}
	bilinear_interpolation(th_gravy, th_gravyf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravx, th_gravxf, XRES/CELL, YRES/CELL, XRES, YRES);
	bilinear_interpolation(th_gravp, th_gravpf, XRES/CELL, YRES/CELL, XRES, YRES);
fin:
	memcpy(th_ogravmap, th_gravmap, sizeof(th_gravmap));
	memset(th_gravmap, 0, sizeof(th_gravmap));
}
#endif



void grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL], char *shapeout)
{
	if(x < 0 || x >= XRES/CELL || y < 0 || y >= YRES/CELL)
		return;
	if(x == 0 || y ==0 || y == (YRES/CELL)-1 || x == (XRES/CELL)-1)
		*shapeout = 1;
	checkmap[y][x] = 1;
	shape[y][x] = 1;
	if(x-1 >= 0 && !checkmap[y][x-1] && bmap[y][x-1]!=WL_GRAV)
		grav_mask_r(x-1, y, checkmap, shape, shapeout);
	if(y-1 >= 0 && !checkmap[y-1][x] && bmap[y-1][x]!=WL_GRAV)
		grav_mask_r(x, y-1, checkmap, shape, shapeout);
	if(x+1 < XRES/CELL && !checkmap[y][x+1] && bmap[y][x+1]!=WL_GRAV)
		grav_mask_r(x+1, y, checkmap, shape, shapeout);
	if(y+1 < YRES/CELL && !checkmap[y+1][x] && bmap[y+1][x]!=WL_GRAV)
		grav_mask_r(x, y+1, checkmap, shape, shapeout);
	return;
}
struct mask_el {
	char *shape;
	char shapeout;
	void *next;
};
typedef struct mask_el mask_el;
void mask_free(mask_el *c_mask_el){
	if(c_mask_el==NULL)
		return;
	if(c_mask_el->next!=NULL)
		mask_free(c_mask_el->next);
	free(c_mask_el->shape);
	free(c_mask_el);
}
void gravity_mask()
{
	char checkmap[YRES/CELL][XRES/CELL];
	int x = 0, y = 0;
	mask_el *t_mask_el = NULL;
	mask_el *c_mask_el = NULL;
	memset(checkmap, 0, sizeof(checkmap));
	for(x = 0; x < XRES/CELL; x++)
	{
		for(y = 0; y < YRES/CELL; y++)
		{
			if(bmap[y][x]!=WL_GRAV && checkmap[y][x] == 0)
			{
				//Create a new shape
				if(t_mask_el==NULL){
					t_mask_el = malloc(sizeof(mask_el));
					t_mask_el->shape = malloc((XRES/CELL)*(YRES/CELL));
					memset(t_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					t_mask_el->shapeout = 0;
					t_mask_el->next = NULL;
					c_mask_el = t_mask_el;
				} else {
					c_mask_el->next = malloc(sizeof(mask_el));
					c_mask_el = c_mask_el->next;
					c_mask_el->shape = malloc((XRES/CELL)*(YRES/CELL));
					memset(c_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					c_mask_el->shapeout = 0;
					c_mask_el->next = NULL;
				}
				//Fill the shape
				grav_mask_r(x, y, checkmap, c_mask_el->shape, &c_mask_el->shapeout);
			}
		}
	}
	c_mask_el = t_mask_el;
	memset(gravmask, 0, sizeof(gravmask));
	while(c_mask_el!=NULL)
	{
		char *cshape = c_mask_el->shape;
		for(x = 0; x < XRES/CELL; x++)
		{
			for(y = 0; y < YRES/CELL; y++)
			{
				if(cshape[y*(XRES/CELL)+x]){
					if(c_mask_el->shapeout)
						gravmask[y][x] = 0xFFFFFFFF;
					else
						gravmask[y][x] = 0x00000000;
				}
			}
		}
		c_mask_el = c_mask_el->next;	
	}
	mask_free(t_mask_el);
}
