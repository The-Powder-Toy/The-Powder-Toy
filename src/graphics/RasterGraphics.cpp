#include "Graphics.h"

#include <cstdlib>
#include <cstring>

Graphics::Graphics():
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

#define VIDXRES WINDOWW
#define VIDYRES WINDOWH
#define PIXELMETHODS_CLASS Graphics
#define DO_CLIPCHECK
#include "RasterDrawMethods.inl"
#undef VIDYRES
#undef VIDXRES
#undef PIXELMETHODS_CLASS
