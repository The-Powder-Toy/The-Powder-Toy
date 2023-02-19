#include "Graphics.h"
#include "SimulationConfig.h"
#include <cstdlib>
#include <cstring>

Graphics::Graphics():
	clip(WINDOW.OriginRect()),
	sdl_scale(1)
{
	vid = (pixel *)malloc(PIXELSIZE * (WINDOWW * WINDOWH));

}

Graphics::~Graphics()
{
	free(vid);
}

void Graphics::Clear()
{
	memset(vid, 0, PIXELSIZE * (WINDOWW * WINDOWH));
}

void Graphics::Finalise()
{

}

constexpr auto VIDXRES = WINDOWW;
constexpr auto VIDYRES = WINDOWH;
#define PIXELMETHODS_CLASS Graphics
#define DO_CLIPCHECK
#include "RasterDrawMethods.inl"
#undef PIXELMETHODS_CLASS
