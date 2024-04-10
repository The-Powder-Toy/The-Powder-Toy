#pragma once
#include "SimulationConfig.h"
#include <cstdint>

constexpr float MAX_TEMP = 9999;
constexpr float MIN_TEMP = 0;
constexpr float O_MAX_TEMP = 3500;
constexpr float O_MIN_TEMP = -273;

constexpr float MAX_PRESSURE = 256.0f;
constexpr float MIN_PRESSURE = -256.0f;

constexpr auto TYPE_PART          = UINT32_C(0x00000001);  //1 Powders
constexpr auto TYPE_LIQUID        = UINT32_C(0x00000002);  //2 Liquids
constexpr auto TYPE_SOLID         = UINT32_C(0x00000004);  //4 Solids
constexpr auto TYPE_GAS           = UINT32_C(0x00000008);  //8 Gases (Includes plasma)
constexpr auto TYPE_ENERGY        = UINT32_C(0x00000010);  //16 Energy (Thunder, Light, Neutrons etc.)
constexpr auto STATE_FLAGS        = UINT32_C(0x0000001F);
constexpr auto PROP_CONDUCTS      = UINT32_C(0x00000020);  //32 Conducts electricity
constexpr auto PROP_PHOTPASS      = UINT32_C(0x00000040);  //64 Photons pass through (may refract as in glass)
constexpr auto PROP_NEUTPENETRATE = UINT32_C(0x00000080);  //128 Penetrated by neutrons
constexpr auto PROP_NEUTABSORB    = UINT32_C(0x00000100);  //256 Absorbs neutrons, reflect is default
constexpr auto PROP_NEUTPASS      = UINT32_C(0x00000200);  //512 Neutrons pass through, such as with glass
constexpr auto PROP_DEADLY        = UINT32_C(0x00000400);  //1024 Is deadly for stickman
constexpr auto PROP_HOT_GLOW      = UINT32_C(0x00000800);  //2048 Hot Metal Glow
constexpr auto PROP_LIFE          = UINT32_C(0x00001000);  //4096 Is a GoL type
constexpr auto PROP_RADIOACTIVE   = UINT32_C(0x00002000);  //8192 Radioactive
constexpr auto PROP_LIFE_DEC      = UINT32_C(0x00004000);  //2^14 Life decreases by one every frame if > zero
constexpr auto PROP_LIFE_KILL     = UINT32_C(0x00008000);  //2^15 Kill when life value is <= zero
constexpr auto PROP_LIFE_KILL_DEC = UINT32_C(0x00010000);  //2^16 Kill when life value is decremented to <= zero
constexpr auto PROP_SPARKSETTLE   = UINT32_C(0x00020000);  //2^17 Allow Sparks/Embers to settle
constexpr auto PROP_NOAMBHEAT     = UINT32_C(0x00040000);  //2^18 Don't transfer or receive heat from ambient heat.
constexpr auto PROP_NOCTYPEDRAW   = UINT32_C(0x00100000); // 2^20 When this element is drawn upon with, do not set ctype (like BCLN for CLNE)

constexpr auto FLAG_STAGNANT      = UINT32_C(0x00000001);
constexpr auto FLAG_SKIPMOVE      = UINT32_C(0x00000002); // skip movement for one frame, only implemented for PHOT
//#define FLAG_WATEREQUAL 0x4 //if a liquid was already checked during equalization
constexpr auto FLAG_MOVABLE       = UINT32_C(0x00000008); // compatibility with old saves (moving SPNG), only applies to SPNG
constexpr auto FLAG_PHOTDECO      = UINT32_C(0x00000008); // compatibility with old saves (decorated photons), only applies to PHOT. Having the same value as FLAG_MOVABLE is fine because they apply to different elements, and this saves space for future flags,


#define UPDATE_FUNC_ARGS Simulation* sim, int i, int x, int y, int surround_space, int nt, Particle *parts, int pmap[YRES][XRES]
#define UPDATE_FUNC_SUBCALL_ARGS sim, i, x, y, surround_space, nt, parts, pmap

#define GRAPHICS_FUNC_ARGS GraphicsFuncContext &gfctx, const Particle *cpart, int nx, int ny, int *pixel_mode, int* cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb
#define GRAPHICS_FUNC_SUBCALL_ARGS gfctx, cpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb

#define ELEMENT_CREATE_FUNC_ARGS Simulation *sim, int i, int x, int y, int t, int v

#define ELEMENT_CREATE_ALLOWED_FUNC_ARGS Simulation *sim, int i, int x, int y, int t

#define ELEMENT_CHANGETYPE_FUNC_ARGS Simulation *sim, int i, int x, int y, int from, int to

#define CTYPEDRAW_FUNC_ARGS Simulation *sim, int i, int t, int v
#define CTYPEDRAW_FUNC_SUBCALL_ARGS sim, i, t, v

constexpr int OLD_PT_WIND = 147;

// Change this to change the amount of bits used to store type in pmap (and a few elements such as PIPE and CRAY)
constexpr int PMAPBITS = 9;
constexpr int PMAPMASK = ((1 << PMAPBITS) - 1);
constexpr int ID(int r)
{
	return r >> PMAPBITS;
}
constexpr int TYP(int r)
{
	return r & PMAPMASK;
}
constexpr int PMAP(int id, int typ)
{
	return (id << PMAPBITS) | (typ & PMAPMASK);
}
constexpr int PMAPID(int id)
{
	return id << PMAPBITS;
}
constexpr int PT_NUM = 1 << PMAPBITS;

constexpr bool InBounds(int x, int y)
{
	return RES.OriginRect().Contains({ x, y });
}

struct playerst;
