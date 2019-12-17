#ifndef ELEMENTCOMMON_H
#define ELEMENTCOMMON_H

#include "Config.h"

// This header should be included by all files in src/elements/

#include <cmath>

#include "Misc.h"

#include "common/tpt-rand.h"
#include "common/tpt-compat.h"
#include "common/tpt-minmax.h"

#include "ElementDefs.h"
#include "ElementClasses.h"
#include "Particle.h"
#include "ElementGraphics.h"
#include "Simulation.h"

#include "graphics/Renderer.h"

#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1

// no transition (PT_NONE means kill part)
#define NT -1

// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM

#endif
