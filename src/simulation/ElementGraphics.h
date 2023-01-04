#pragma once
#include <cstdint>

constexpr auto PMODE           = UINT32_C(0x00000FFF);
constexpr auto PMODE_NONE      = UINT32_C(0x00000000);
constexpr auto PMODE_FLAT      = UINT32_C(0x00000001);
constexpr auto PMODE_BLOB      = UINT32_C(0x00000002);
constexpr auto PMODE_BLUR      = UINT32_C(0x00000004);
constexpr auto PMODE_GLOW      = UINT32_C(0x00000008);
constexpr auto PMODE_SPARK     = UINT32_C(0x00000010);
constexpr auto PMODE_FLARE     = UINT32_C(0x00000020);
constexpr auto PMODE_LFLARE    = UINT32_C(0x00000040);
constexpr auto PMODE_ADD       = UINT32_C(0x00000080);
constexpr auto PMODE_BLEND     = UINT32_C(0x00000100);
constexpr auto PSPEC_STICKMAN  = UINT32_C(0x00000200);

constexpr auto OPTIONS         = UINT32_C(0x0000F000);
constexpr auto NO_DECO         = UINT32_C(0x00001000);
constexpr auto DECO_FIRE       = UINT32_C(0x00002000);

constexpr auto FIREMODE        = UINT32_C(0x00FF0000);
constexpr auto FIRE_ADD        = UINT32_C(0x00010000);
constexpr auto FIRE_BLEND      = UINT32_C(0x00020000);
constexpr auto FIRE_SPARK      = UINT32_C(0x00040000);

constexpr auto EFFECT          = UINT32_C(0xFF000000);
constexpr auto EFFECT_GRAVIN   = UINT32_C(0x01000000);
constexpr auto EFFECT_GRAVOUT  = UINT32_C(0x02000000);
constexpr auto EFFECT_LINES    = UINT32_C(0x04000000);
constexpr auto EFFECT_DBGLINES = UINT32_C(0x08000000);

constexpr auto RENDER_EFFE     = OPTIONS | PSPEC_STICKMAN | EFFECT | PMODE_SPARK | PMODE_FLARE | PMODE_LFLARE;
constexpr auto RENDER_FIRE     = OPTIONS | PSPEC_STICKMAN | PMODE_ADD | PMODE_BLEND | FIRE_ADD | FIRE_BLEND;
constexpr auto RENDER_SPRK     = OPTIONS | PSPEC_STICKMAN | PMODE_ADD | PMODE_BLEND | FIRE_SPARK;
constexpr auto RENDER_GLOW     = OPTIONS | PSPEC_STICKMAN | PMODE_GLOW | PMODE_ADD | PMODE_BLEND;
constexpr auto RENDER_BLUR     = OPTIONS | PSPEC_STICKMAN | PMODE_BLUR | PMODE_ADD | PMODE_BLEND;
constexpr auto RENDER_BLOB     = OPTIONS | PSPEC_STICKMAN | PMODE_BLOB | PMODE_ADD | PMODE_BLEND;
constexpr auto RENDER_BASC     = OPTIONS | PSPEC_STICKMAN | PMODE_FLAT | PMODE_ADD | PMODE_BLEND | EFFECT_LINES;
constexpr auto RENDER_NONE     = OPTIONS | PSPEC_STICKMAN | PMODE_FLAT;

constexpr auto COLOUR_HEAT     = UINT32_C(0x00000001);
constexpr auto COLOUR_LIFE     = UINT32_C(0x00000002);
constexpr auto COLOUR_GRAD     = UINT32_C(0x00000004);
constexpr auto COLOUR_BASC     = UINT32_C(0x00000008);

constexpr auto COLOUR_DEFAULT  = UINT32_C(0x00000000);

constexpr auto DISPLAY_AIRC    = UINT32_C(0x00000001);
constexpr auto DISPLAY_AIRP    = UINT32_C(0x00000002);
constexpr auto DISPLAY_AIRV    = UINT32_C(0x00000004);
constexpr auto DISPLAY_AIRH    = UINT32_C(0x00000008);
constexpr auto DISPLAY_AIR     = UINT32_C(0x0000000F);
constexpr auto DISPLAY_WARP    = UINT32_C(0x00000010);
constexpr auto DISPLAY_PERS    = UINT32_C(0x00000020);
constexpr auto DISPLAY_EFFE    = UINT32_C(0x00000040);
