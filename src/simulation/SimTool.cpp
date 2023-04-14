#include "common/tpt-rand.h"
#include "graphics/Renderer.h"
#include "simulation/ElementGraphics.h"
#include "simulation/gravity/Gravity.h"
#include "simulation/Simulation.h"
#include "Misc.h"
#include "ToolClasses.h"
#include <cmath>

SimTool::SimTool():
Identifier("DEFAULT_TOOL_INVALID"),
Name(""),
Colour(0xFFFFFF_rgb),
Description("NULL Tool, does NOTHING")
{
}
