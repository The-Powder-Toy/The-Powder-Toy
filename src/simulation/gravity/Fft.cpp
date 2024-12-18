#include "Gravity.h"
#include "Config.h"
#include "SimulationConfig.h"
#include <cstring>
#include <cmath>
#include <complex>
#include <memory>
#include <fftw3.h>
#include <thread>
#include <mutex>
#include <condition_variable>

// DFT is cyclic in nature; gravity would wrap around sort of like in loop mode without the 2x here;
// in fact it still does, it's just not as visible. the arrays are 2x as big along all dimensions as normal cell maps
constexpr auto blocks = CELLS * 2;

// https://www.fftw.org/fftw3_doc/Multi_002dDimensional-DFTs-of-Real-Data.html#Multi_002dDimensional-DFTs-of-Real-Data
constexpr auto transSize = (blocks.X / 2 + 1) * blocks.Y;

// NCELL * 4 is size of data array, scaling needed because FFTW calculates an unnormalized DFT
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
	FftwArrayPtr                            massBig , forceXBig , forceYBig ;
	FftwComplexArrayPtr kernelXT, kernelYT, massBigT, forceXBigT, forceYBigT;
	FftwPlanPtr massForward, forceXInverse, forceYInverse;
	bool initDone = false;

	std::thread thr;
	bool working = false;
	bool shouldStop = false;
	std::mutex stateMx;
	std::condition_variable stateCv;

	GravityInput gravIn;
	GravityOutput gravOut;
	bool copyGravOut = false;

	~GravityImpl();

	void Init();
	void Work();
	void Wait();
	void Stop();
	void Dispatch();
};

GravityImpl::~GravityImpl()
{
	if (initDone)
	{
		Wait();
		Stop();
	}
}

void GravityImpl::Dispatch()
{
	{
		std::unique_lock lk(stateMx);
		working = true;
	}
	stateCv.notify_one();
}

void GravityImpl::Stop()
{
	{
		std::unique_lock lk(stateMx);
		shouldStop = true;
	}
	stateCv.notify_one();
	thr.join();
}

void GravityImpl::Wait()
{
	std::unique_lock lk(stateMx);
	stateCv.wait(lk, [this]() {
		return !working;
	});
}

void GravityImpl::Work()
{
	{
		PlaneAdapter<PlaneBase<float>, blocks.X, blocks.Y> massBigP(blocks, std::in_place, massBig.get());
		for (auto p : CELLS.OriginRect())
		{
			// used to be a membwand but we'd need a new buffer for this,
			// not worth it just to make this unalinged copy faster
			massBigP[p + CELLS] = gravIn.mask[p] ? gravIn.mass[p] : 0.f;
		}
	}
	fftwf_execute(massForward.get());
	{
		// https://en.wikipedia.org/wiki/Convolution_theorem
		for (int i = 0; i < transSize; ++i)
		{
			forceXBigT[i] = massBigT[i] * kernelXT[i];
			forceYBigT[i] = massBigT[i] * kernelYT[i];
		}
	}
	fftwf_execute(forceXInverse.get());
	fftwf_execute(forceYInverse.get());
	{
		PlaneAdapter<PlaneBase<float>, blocks.X, blocks.Y> forceXBigP(blocks, std::in_place, forceXBig.get());
		PlaneAdapter<PlaneBase<float>, blocks.X, blocks.Y> forceYBigP(blocks, std::in_place, forceYBig.get());
		for (auto p : CELLS.OriginRect())
		{
			// similarly
			gravOut.forceX[p] = gravIn.mask[p] ? forceXBigP[p] : 0;
			gravOut.forceY[p] = gravIn.mask[p] ? forceYBigP[p] : 0;
		}
	}
}

void GravityImpl::Init()
{
	//select best algorithm, could use FFTW_PATIENT or FFTW_EXHAUSTIVE but that increases the time taken to plan, and I don't see much increase in execution speed
	auto fftwPlanFlags = FFTW_PLAN_MEASURE ? FFTW_MEASURE : FFTW_ESTIMATE;

	//use fftw malloc function to ensure arrays are aligned, to get better performance
	kernelXT = FftwComplexArray(transSize);
	kernelYT = FftwComplexArray(transSize);
	massBig = FftwArray(blocks.X * blocks.Y);
	massBigT = FftwComplexArray(transSize);
	forceXBig = FftwArray(blocks.X * blocks.Y);
	forceYBig = FftwArray(blocks.X * blocks.Y);
	forceXBigT = FftwComplexArray(transSize);
	forceYBigT = FftwComplexArray(transSize);

	massForward = FftwPlanPtr(fftwf_plan_dft_r2c_2d(blocks.Y, blocks.X, massBig.get(), reinterpret_cast<fftwf_complex *>(massBigT.get()), fftwPlanFlags));
	forceXInverse = FftwPlanPtr(fftwf_plan_dft_c2r_2d(blocks.Y, blocks.X, reinterpret_cast<fftwf_complex *>(forceXBigT.get()), forceXBig.get(), fftwPlanFlags));
	forceYInverse = FftwPlanPtr(fftwf_plan_dft_c2r_2d(blocks.Y, blocks.X, reinterpret_cast<fftwf_complex *>(forceYBigT.get()), forceYBig.get(), fftwPlanFlags));

	auto kernelXRaw = FftwArray(blocks.X * blocks.Y);
	auto kernelYRaw = FftwArray(blocks.X * blocks.Y);
	auto kernelXForward = FftwPlanPtr(fftwf_plan_dft_r2c_2d(blocks.Y, blocks.X, kernelXRaw.get(), reinterpret_cast<fftwf_complex *>(kernelXT.get()), fftwPlanFlags));
	auto kernelYForward = FftwPlanPtr(fftwf_plan_dft_r2c_2d(blocks.Y, blocks.X, kernelYRaw.get(), reinterpret_cast<fftwf_complex *>(kernelYT.get()), fftwPlanFlags));
	PlaneAdapter<PlaneBase<float>, blocks.X, blocks.Y> kernelX(blocks, std::in_place, kernelXRaw.get());
	PlaneAdapter<PlaneBase<float>, blocks.X, blocks.Y> kernelY(blocks, std::in_place, kernelYRaw.get());
	//calculate velocity map caused by a point mass
	for (auto p : blocks.OriginRect())
	{
		auto d = p - CELLS;
		if (d == Vec2{ 0, 0 })
		{
			kernelX[p] = 0.f;
			kernelY[p] = 0.f;
		}
		else
		{
			auto distance = std::hypot(float(d.X), float(d.Y));
			kernelX[p] = scaleFactor * d.X / std::pow(distance, 3.f);
			kernelY[p] = scaleFactor * d.Y / std::pow(distance, 3.f);
		}
	}

	//transform point mass velocity maps
	fftwf_execute(kernelXForward.get());
	fftwf_execute(kernelYForward.get());

	//clear padded gravmap
	std::fill(massBig.get(), massBig.get() + blocks.X * blocks.Y, 0.f);

	thr = std::thread([this]() {
		while (true)
		{
			{
				std::unique_lock lk(stateMx);
				stateCv.wait(lk, [this]() {
					return working || shouldStop;
				});
				if (shouldStop)
				{
					break;
				}
			}
			Work();
			{
				std::unique_lock lk(stateMx);
				working = false;
			}
			stateCv.notify_one();
		}
	});
}

void Gravity::Exchange(GravityOutput &gravOut, GravityInput &gravIn, bool forceRecalc)
{
	auto *fftGravity = static_cast<GravityImpl *>(this);

	// lazy init
	if (!fftGravity->initDone)
	{
		// this takes a noticeable amount of time
		// TODO: hide the wait somehow
		fftGravity->Init();
		fftGravity->initDone = true;
	}

	fftGravity->Wait();

	// take output
	if (fftGravity->copyGravOut)
	{
		fftGravity->copyGravOut = false;
		std::swap(gravOut, fftGravity->gravOut);
	}

	// pass input (but same input => same output)
	if (forceRecalc ||
	    std::memcmp(&fftGravity->gravIn.mass[{ 0, 0 }], &gravIn.mass[{ 0, 0 }], NCELL * sizeof(float)) ||
	    std::memcmp(&fftGravity->gravIn.mask[{ 0, 0 }], &gravIn.mask[{ 0, 0 }], NCELL * sizeof(float)))
	{
		fftGravity->copyGravOut = true;
		std::swap(gravIn, fftGravity->gravIn);
		fftGravity->Dispatch();
	}
}

GravityPtr Gravity::Create()
{
	return GravityPtr(new GravityImpl());
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete static_cast<GravityImpl *>(ptr);
}
