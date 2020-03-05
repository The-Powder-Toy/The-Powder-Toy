#include <cmath>
#include "simulation/ElementGraphics.h"
#include "simulation/Gravity.h"
#include "Misc.h"
#include "simulation/Simulation.h"
#include "common/tpt-compat.h"
#include "common/tpt-rand.h"
#include "graphics/Renderer.h"

#include "ToolClasses.h"

SimTool::SimTool():
Identifier("DEFAULT_TOOL_INVALID"),
Name(""),
Colour(PIXPACK(0xFFFFFF)),
Description("NULL Tool, does NOTHING")
{
}
