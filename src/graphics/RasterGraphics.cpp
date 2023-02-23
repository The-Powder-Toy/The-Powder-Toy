#include "Graphics.h"
#include "RasterDrawMethodsImpl.h"
#include "SimulationConfig.h"
#include <cstdlib>
#include <cstring>

Graphics::Graphics():
	clip(WINDOW.OriginRect()),
	vid(std::make_unique<pixel []>(WINDOWW * WINDOWH))
{
}

void Graphics::Clear()
{
	std::fill_n(vid.Base.get(), WINDOWW * WINDOWH, 0);
}

void Graphics::Finalise()
{

}

template class RasterDrawMethods<Graphics>;
