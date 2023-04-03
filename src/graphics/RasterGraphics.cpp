#include <cstdlib>
#include <cstring>
#include "Graphics.h"
#include "SimulationConfig.h"
#include "RasterDrawMethodsImpl.h"

void Graphics::Clear()
{
	memset(vid, 0, PIXELSIZE * (WINDOWW * WINDOWH));
}

void Graphics::Finalise()
{

}

template class RasterDrawMethods<Graphics>;
