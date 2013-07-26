#include "Graphics.h"

#ifndef OGLI

Graphics::Graphics():
sdl_scale(1)
{
	vid = (pixel *)malloc(PIXELSIZE * ((XRES+BARSIZE) * (YRES+MENUSIZE)));

}

void Graphics::Acquire()
{

}

void Graphics::Release()
{

}

Graphics::~Graphics()
{
	free(vid);
}

void Graphics::Clear()
{
	memset(vid, 0, PIXELSIZE * ((XRES+BARSIZE) * (YRES+MENUSIZE)));
}

void Graphics::Finalise()
{

}

#define VIDXRES XRES+BARSIZE
#define VIDYRES YRES+MENUSIZE
#define PIXELMETHODS_CLASS Graphics
#include "RasterDrawMethods.inl"
#undef VIDYRES
#undef VIDXRES
#undef PIXELMETHODS_CLASS

#endif
