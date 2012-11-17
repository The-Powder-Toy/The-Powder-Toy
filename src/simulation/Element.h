#ifndef ELEMENT_H
#define ELEMENT_H
// This header should be included by all files in src/elements/

#include <cmath>
#include "Simulation.h"
#include "graphics/Renderer.h"
#include "Gravity.h"
#include "Misc.h"
#include "ElementGraphics.h"

#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1

// no transition (PT_NONE means kill part)
#define NT -1

// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM

#endif
