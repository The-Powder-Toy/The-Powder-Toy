#pragma once
#include "simulation/ElementDefs.h"

int Element_PIPE_graphics(GRAPHICS_FUNC_ARGS);
void Element_PIPE_transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR);
void Element_PIPE_transformPatchOffsets(Particle &part, const std::array<int, 8> &offsetMap);
int Element_PIPE_update(UPDATE_FUNC_ARGS);
void Element_PPIP_flood_trigger(Simulation * sim, int x, int y, int sparkedBy);

extern int Element_PPIP_ppip_changed;
extern const std::array<Vec2<int>, 8> Element_PIPE_offsets;
