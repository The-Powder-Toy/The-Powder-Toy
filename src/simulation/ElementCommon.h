#pragma once
// This header should be included by all files in src/elements/
#include "Misc.h"
#include "common/tpt-rand.h"
#include "common/tpt-compat.h"
#include "ElementDefs.h"
#include "ElementClasses.h"
#include "Particle.h"
#include "ElementGraphics.h"
#include "Simulation.h"
#include "graphics/Renderer.h"
#include <algorithm>
#include <cmath>

constexpr float IPL = MIN_PRESSURE - 1;
constexpr float IPH = MAX_PRESSURE + 1;
constexpr float ITL = MIN_TEMP - 1;
constexpr float ITH = MAX_TEMP + 1;

// no transition (PT_NONE means kill part)
constexpr int NT = -1;

// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
constexpr int ST = PT_NUM;
