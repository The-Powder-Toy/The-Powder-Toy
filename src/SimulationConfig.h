#pragma once
#include <cstdint>
#include <common/Vec2.h>

constexpr int MENUSIZE = 40;
constexpr int BARSIZE  = 17;

constexpr float M_GRAV = 6.67300e-1f;

//CELL, the size of the pressure, gravity, and wall maps. Larger than 1 to prevent extreme lag
constexpr int CELL = 4;
constexpr Vec2<int> CELLS = Vec2(153, 96);
constexpr Vec2<int> RES = CELLS * CELL;

constexpr int XCELLS = CELLS.X;
constexpr int YCELLS = CELLS.Y;
constexpr int NCELL  = XCELLS * YCELLS;
constexpr int XRES   = RES.X;
constexpr int YRES   = RES.Y;
constexpr int NPART  = XRES * YRES;

constexpr int XCNTR = XRES / 2;
constexpr int YCNTR = YRES / 2;

constexpr Vec2<int> WINDOW = RES + Vec2(BARSIZE, MENUSIZE);

constexpr int WINDOWW = WINDOW.X;
constexpr int WINDOWH = WINDOW.Y;

constexpr int MAXSIGNS = 16;

constexpr int   ISTP            = CELL / 2;
constexpr float CFDS            = 4.0f / CELL;
constexpr float MAX_VELOCITY = 1e4f;

//Air constants
constexpr float AIR_TSTEPP = 0.3f;
constexpr float AIR_TSTEPV = 0.4f;
constexpr float AIR_VADV   = 0.3f;
constexpr float AIR_VLOSS  = 0.999f;
constexpr float AIR_PLOSS  = 0.9999f;

constexpr int NGOL = 24;

enum DefaultBrushes
{
	BRUSH_CIRCLE,
	BRUSH_SQUARE,
	BRUSH_TRIANGLE,
	NUM_DEFAULTBRUSHES,
};

//Photon constants
constexpr int SURF_RANGE     = 10;
constexpr int NORMAL_MIN_EST =  3;
constexpr int NORMAL_INTERP  = 20;
constexpr int NORMAL_FRAC    = 16;

constexpr auto REFRACT = UINT32_C(0x80000000);

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
constexpr float GLASS_IOR  = 1.9f;
constexpr float GLASS_DISP = 0.07f;

constexpr float R_TEMP = 22;

constexpr bool LATENTHEAT = false;
