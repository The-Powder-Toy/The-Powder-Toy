#include <cmath>

#include "common/tpt-rand.h"
#include "graphics/Renderer.h"
#include "simulation/ElementGraphics.h"
#include "simulation/Gravity.h"
#include "simulation/Simulation.h"

#include "Misc.h"
#include "ToolClasses.h"

SimTool::SimTool():
Identifier("DEFAULT_TOOL_INVALID"),
Name(""),
Colour(PIXPACK(0xFFFFFF)),
Description("NULL Tool, does NOTHING")
{
}
