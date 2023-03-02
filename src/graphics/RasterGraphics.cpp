#include "Graphics.h"
#include "RasterDrawMethodsImpl.h"
#include "SimulationConfig.h"
#include <cstdlib>
#include <cstring>

Graphics::Graphics():
	clip(WINDOW.OriginRect())
{
}

void Graphics::Clear()
{
	std::fill(std::begin(vid), std::end(vid), 0);
}

void Graphics::Finalise()
{

}

template class RasterDrawMethods<Graphics>;
