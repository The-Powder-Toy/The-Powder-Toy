#pragma once
#include "ElementDefs.h"

constexpr float IPL = MIN_PRESSURE - 1;
constexpr float IPH = MAX_PRESSURE + 1;
constexpr float ITL = MIN_TEMP - 1;
constexpr float ITH = MAX_TEMP + 1;

// no transition (PT_NONE means kill part)
constexpr int NT = -1;

// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
constexpr int ST = PT_NUM;
