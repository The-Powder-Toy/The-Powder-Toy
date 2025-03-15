#pragma once
#include "simulation/ElementDefs.h"

int Element_PIPE_graphics(GRAPHICS_FUNC_ARGS);
void Element_PIPE_transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR);
void Element_PIPE_transfer_part_to_pipe(Particle *part, Particle *pipe);
void Element_PIPE_transformPatchOffsets(Particle &part, const std::array<int, 8> &offsetMap);
int Element_PIPE_update(UPDATE_FUNC_ARGS);
void Element_PPIP_flood_trigger(Simulation * sim, int x, int y, int sparkedBy);

// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction
// 0x000E0000 PIPE color data stored here

constexpr int PFLAG_CAN_CONDUCT            = 0x00000001;
constexpr int PFLAG_PARTICLE_DECO          = 0x00000002; // differentiate particle deco from pipe deco
constexpr int PFLAG_NORMALSPEED            = 0x00010000;
constexpr int PFLAG_INITIALIZING           = 0x00020000; // colors haven't been set yet
constexpr int PFLAG_COLOR_RED              = 0x00040000;
constexpr int PFLAG_COLOR_GREEN            = 0x00080000;
constexpr int PFLAG_COLOR_BLUE             = 0x000C0000;
constexpr int PFLAG_COLORS                 = 0x000C0000;

extern int Element_PPIP_ppip_changed;
extern const std::array<Vec2<int>, 8> Element_PIPE_offsets;
