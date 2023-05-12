#include <cstdlib>
#include <cstring>
#include "Graphics.h"
#include "SimulationConfig.h"
#include "RasterDrawMethodsImpl.h"

void Graphics::Finalise()
{

}

template struct RasterDrawMethods<Graphics>;
