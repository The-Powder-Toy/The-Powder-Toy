#include "Gravity.h"
#include "Misc.h"
#include <cstring>
#include <cmath>
#include <fftw3.h>

struct GravityImpl : public Gravity
{
	bool grav_fft_status = false;
	float *th_ptgravx = nullptr;
	float *th_ptgravy = nullptr;
	float *th_gravmapbig = nullptr;
	float *th_gravxbig = nullptr;
	float *th_gravybig = nullptr;

	fftwf_complex *th_ptgravxt, *th_ptgravyt, *th_gravmapbigt, *th_gravxbigt, *th_gravybigt;
	fftwf_plan plan_gravmap, plan_gravx_inverse, plan_gravy_inverse;

	void grav_fft_init();
	void grav_fft_cleanup();

	GravityImpl() : Gravity(CtorTag{})
	{
	}

	~GravityImpl();
};

GravityImpl::~GravityImpl()
{
	stop_grav_async();
	grav_fft_cleanup();
}

void GravityImpl::grav_fft_init()
{
	int xblock2 = XCELLS*2;
	int yblock2 = YCELLS*2;
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

	//NCELL*4 is size of data array, scaling needed because FFTW calculates an unnormalized DFT
	scaleFactor = -float(M_GRAV)/(NCELL*4);
	//calculate velocity map caused by a point mass
	for (int y = 0; y < yblock2; y++)
	{
		for (int x = 0; x < xblock2; x++)
		{
			if (x == XCELLS && y == YCELLS)
				continue;
			distance = hypotf(float(x-XCELLS), float(y-YCELLS));
			th_ptgravx[y * xblock2 + x] = scaleFactor * (x - XCELLS) / powf(distance, 3);
			th_ptgravy[y * xblock2 + x] = scaleFactor * (y - YCELLS) / powf(distance, 3);
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

void GravityImpl::grav_fft_cleanup()
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

void Gravity::get_result()
{
	std::swap(gravy, th_gravy);
	std::swap(gravx, th_gravx);
	std::swap(gravp, th_gravp);
}

void Gravity::update_grav()
{
	auto *fftGravity = static_cast<GravityImpl *>(this);
	if (!fftGravity->grav_fft_status)
		fftGravity->grav_fft_init();

	auto *th_gravmapbig = fftGravity->th_gravmapbig;
	auto *th_gravxbig = fftGravity->th_gravxbig;
	auto *th_gravybig = fftGravity->th_gravybig;
	auto *th_ptgravxt = fftGravity->th_ptgravxt;
	auto *th_ptgravyt = fftGravity->th_ptgravyt;
	auto *th_gravmapbigt = fftGravity->th_gravmapbigt;
	auto *th_gravxbigt = fftGravity->th_gravxbigt;
	auto *th_gravybigt = fftGravity->th_gravybigt;
	auto &plan_gravmap = fftGravity->plan_gravmap;
	auto &plan_gravx_inverse = fftGravity->plan_gravx_inverse;
	auto &plan_gravy_inverse = fftGravity->plan_gravy_inverse;

	int xblock2 = XCELLS*2, yblock2 = YCELLS*2;
	int fft_tsize = (xblock2/2+1)*yblock2;
	float mr, mc, pr, pc, gr, gc;
	if (memcmp(th_ogravmap, th_gravmap, sizeof(float)*NCELL) != 0)
	{
		th_gravchanged = 1;

		membwand(th_gravmap, gravmask, NCELL*sizeof(float), NCELL*sizeof(unsigned));
		//copy gravmap into padded gravmap array
		for (int y = 0; y < YCELLS; y++)
		{
			for (int x = 0; x < XCELLS; x++)
			{
				th_gravmapbig[(y+YCELLS)*xblock2+XCELLS+x] = th_gravmap[y*XCELLS+x];
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
		for (int y = 0; y < YCELLS; y++)
		{
			for (int x = 0; x < XCELLS; x++)
			{
				th_gravx[y*XCELLS+x] = th_gravxbig[y*xblock2+x];
				th_gravy[y*XCELLS+x] = th_gravybig[y*xblock2+x];
				th_gravp[y*XCELLS+x] = hypotf(th_gravxbig[y*xblock2+x], th_gravybig[y*xblock2+x]);
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

GravityPtr Gravity::Create()
{
	return GravityPtr(new GravityImpl());
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete static_cast<GravityImpl *>(ptr);
}
