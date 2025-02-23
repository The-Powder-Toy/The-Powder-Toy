#pragma once
#include "simulation/ElementDefs.h"

int Element_FIGH_Alloc(Simulation *sim);
bool Element_FIGH_CanAlloc(Simulation *sim);
void Element_FIGH_NewFighter(Simulation *sim, int fighterID, int i, int elem);
int Element_STKM_graphics(GRAPHICS_FUNC_ARGS);
void Element_STKM_init_legs(Simulation * sim, playerst *playerp, int i);
void Element_STKM_interact(Simulation *sim, playerst *playerp, int i, int x, int y);
int Element_STKM_run_stickman(playerst *playerp, UPDATE_FUNC_ARGS);
void Element_STKM_set_element(Simulation *sim, playerst *playerp, int element);
