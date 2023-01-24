#include "Gravity.h"
#include "Misc.h"
#include <cstring>
#include <cmath>
#include <complex>
#include <fftw3.h>

constexpr auto xblock2     = XCELLS * 2;
constexpr auto yblock2     = YCELLS * 2;
constexpr auto fft_tsize   = (xblock2 / 2 + 1) * yblock2;
//NCELL*4 is size of data array, scaling needed because FFTW calculates an unnormalized DFT
constexpr auto scaleFactor = -float(M_GRAV) / (NCELL * 4);

static_assert(sizeof(std::complex<float>) == sizeof(fftwf_complex));
struct FftwArrayDeleter        { void operator ()(float               ptr[]) const { fftwf_free(ptr);         } };
struct FftwComplexArrayDeleter { void operator ()(std::complex<float> ptr[]) const { fftwf_free(ptr);         } };
struct FftwPlanDeleter         { void operator ()(fftwf_plan          ptr  ) const { fftwf_destroy_plan(ptr); } };
using  FftwArrayPtr        = std::unique_ptr<float                              [], FftwArrayDeleter       >;
using  FftwComplexArrayPtr = std::unique_ptr<std::complex<float>                [], FftwComplexArrayDeleter>;
using  FftwPlanPtr         = std::unique_ptr<std::remove_pointer<fftwf_plan>::type, FftwPlanDeleter        >;
FftwArrayPtr FftwArray(size_t size)
{
	return FftwArrayPtr(reinterpret_cast<float *>(fftwf_malloc(size * sizeof(float))));
}
FftwComplexArrayPtr FftwComplexArray(size_t size)
{
	return FftwComplexArrayPtr(reinterpret_cast<std::complex<float> *>(fftwf_malloc(size * sizeof(std::complex<float>))));
}

struct GravityImpl : public Gravity
{
	bool grav_fft_status = false;
	FftwArrayPtr                                  th_gravmapbig , th_gravxbig , th_gravybig ;
	FftwComplexArrayPtr th_ptgravxt, th_ptgravyt, th_gravmapbigt, th_gravxbigt, th_gravybigt;
	FftwPlanPtr plan_gravmap, plan_gravx_inverse, plan_gravy_inverse;

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
	if (grav_fft_status) return;
	FftwPlanPtr plan_ptgravx, plan_ptgravy;

	//use fftw malloc function to ensure arrays are aligned, to get better performance
	FftwArrayPtr th_ptgravx = FftwArray(xblock2 * yblock2);
	FftwArrayPtr th_ptgravy = FftwArray(xblock2 * yblock2);
	th_ptgravxt = FftwComplexArray(fft_tsize);
	th_ptgravyt = FftwComplexArray(fft_tsize);
	th_gravmapbig = FftwArray(xblock2 * yblock2);
	th_gravmapbigt = FftwComplexArray(fft_tsize);
	th_gravxbig = FftwArray(xblock2 * yblock2);
	th_gravybig = FftwArray(xblock2 * yblock2);
	th_gravxbigt = FftwComplexArray(fft_tsize);
	th_gravybigt = FftwComplexArray(fft_tsize);

	//select best algorithm, could use FFTW_PATIENT or FFTW_EXHAUSTIVE but that increases the time taken to plan, and I don't see much increase in execution speed
	auto fftwPlanFlags = FFTW_PLAN_MEASURE ? FFTW_MEASURE : FFTW_ESTIMATE;
	plan_ptgravx = FftwPlanPtr(fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravx.get(), reinterpret_cast<fftwf_complex *>(th_ptgravxt.get()), fftwPlanFlags));
	plan_ptgravy = FftwPlanPtr(fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_ptgravy.get(), reinterpret_cast<fftwf_complex *>(th_ptgravyt.get()), fftwPlanFlags));
	plan_gravmap = FftwPlanPtr(fftwf_plan_dft_r2c_2d(yblock2, xblock2, th_gravmapbig.get(), reinterpret_cast<fftwf_complex *>(th_gravmapbigt.get()), fftwPlanFlags));
	plan_gravx_inverse = FftwPlanPtr(fftwf_plan_dft_c2r_2d(yblock2, xblock2, reinterpret_cast<fftwf_complex *>(th_gravxbigt.get()), th_gravxbig.get(), fftwPlanFlags));
	plan_gravy_inverse = FftwPlanPtr(fftwf_plan_dft_c2r_2d(yblock2, xblock2, reinterpret_cast<fftwf_complex *>(th_gravybigt.get()), th_gravybig.get(), fftwPlanFlags));

	//calculate velocity map caused by a point mass
	for (int y = 0; y < yblock2; y++)
	{
		for (int x = 0; x < xblock2; x++)
		{
			if (x == XCELLS && y == YCELLS)
				continue;
			auto distance = hypotf(float(x-XCELLS), float(y-YCELLS));
			th_ptgravx[y * xblock2 + x] = scaleFactor * (x - XCELLS) / powf(distance, 3);
			th_ptgravy[y * xblock2 + x] = scaleFactor * (y - YCELLS) / powf(distance, 3);
		}
	}
	th_ptgravx[yblock2 * xblock2 / 2 + xblock2 / 2] = 0.0f;
	th_ptgravy[yblock2 * xblock2 / 2 + xblock2 / 2] = 0.0f;

	//transform point mass velocity maps
	fftwf_execute(plan_ptgravx.get());
	fftwf_execute(plan_ptgravy.get());

	//clear padded gravmap
	memset(th_gravmapbig.get(), 0, xblock2 * yblock2 * sizeof(float));

	grav_fft_status = true;
}

void GravityImpl::grav_fft_cleanup()
{
	if (!grav_fft_status) return;
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

	auto *th_gravmapbig = fftGravity->th_gravmapbig.get();
	auto *th_gravxbig = fftGravity->th_gravxbig.get();
	auto *th_gravybig = fftGravity->th_gravybig.get();
	auto *th_ptgravxt = fftGravity->th_ptgravxt.get();
	auto *th_ptgravyt = fftGravity->th_ptgravyt.get();
	auto *th_gravmapbigt = fftGravity->th_gravmapbigt.get();
	auto *th_gravxbigt = fftGravity->th_gravxbigt.get();
	auto *th_gravybigt = fftGravity->th_gravybigt.get();
	auto &plan_gravmap = fftGravity->plan_gravmap;
	auto &plan_gravx_inverse = fftGravity->plan_gravx_inverse;
	auto &plan_gravy_inverse = fftGravity->plan_gravy_inverse;

	if (memcmp(&th_ogravmap[0], &th_gravmap[0], sizeof(float) * NCELL) != 0)
	{
		th_gravchanged = 1;

		membwand(&th_gravmap[0], &gravmask[0], NCELL * sizeof(float), NCELL * sizeof(uint32_t));
		//copy gravmap into padded gravmap array
		for (int y = 0; y < YCELLS; y++)
		{
			for (int x = 0; x < XCELLS; x++)
			{
				th_gravmapbig[(y+YCELLS)*xblock2+XCELLS+x] = th_gravmap[y*XCELLS+x];
			}
		}
		//transform gravmap
		fftwf_execute(plan_gravmap.get());
		//do convolution (multiply the complex numbers)
		for (int i = 0; i < fft_tsize; i++)
		{
			th_gravxbigt[i] = th_gravmapbigt[i] * th_ptgravxt[i];
			th_gravybigt[i] = th_gravmapbigt[i] * th_ptgravyt[i];
		}
		//inverse transform, and copy from padded arrays into normal velocity maps
		fftwf_execute(plan_gravx_inverse.get());
		fftwf_execute(plan_gravy_inverse.get());
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
