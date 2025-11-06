#pragma once
#include "simulation/ElementDefs.h"

int Element_PAPR_update(UPDATE_FUNC_ARGS);
int Element_PAPR_graphics(GRAPHICS_FUNC_ARGS);
bool Element_PAPR_ctypeDraw(CTYPEDRAW_FUNC_ARGS);

constexpr unsigned int MARK_COLOR_COAL = 0xFF22222A;
