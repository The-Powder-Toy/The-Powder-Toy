#pragma once
#include <cstdint>

// Boolean macros (defined / not defined), would be great to get rid of them all.
#mesondefine NOHTTP
#mesondefine GRAVFFT
#mesondefine RENDERER
#mesondefine FONTEDITOR
#mesondefine BETA
#mesondefine DEBUG
#mesondefine LIN
#mesondefine AND
#mesondefine SNAPSHOT
#mesondefine MOD
#mesondefine WIN
#mesondefine MACOSX
#mesondefine X86

constexpr bool LUACONSOLE               = @LUACONSOLE@;
constexpr bool ALLOW_FAKE_NEWER_VERSION = @ALLOW_FAKE_NEWER_VERSION@;
constexpr bool USE_UPDATESERVER         = @USE_UPDATESERVER@;
constexpr bool INSTALL_CHECK            = @INSTALL_CHECK@;
constexpr bool IGNORE_UPDATES           = @IGNORE_UPDATES@;
constexpr bool ENFORCE_HTTPS            = @ENFORCE_HTTPS@;

constexpr char SERVER[]         = "@SERVER@";
constexpr char STATICSERVER[]   = "@STATICSERVER@";
constexpr char UPDATESERVER[]   = "@UPDATESERVER@";
constexpr char IDENT_PLATFORM[] = "@IDENT_PLATFORM@";
constexpr char IDENT[]          = "@IDENT@";
constexpr char APPNAME[]        = "@APPNAME@";
constexpr char APPCOMMENT[]     = "@APPCOMMENT@";
constexpr char APPEXE[]         = "@APPEXE@";
constexpr char APPID[]          = "@APPID@";
constexpr char APPDATA[]        = "@APPDATA@";
constexpr char APPVENDOR[]      = "@APPVENDOR@";

#ifdef WIN
constexpr char PATH_SEP[] = "\\";
constexpr char PATH_SEP_CHAR = '\\';
#else
constexpr char PATH_SEP[] = "/";
constexpr char PATH_SEP_CHAR = '/';
#endif

//VersionInfoStart
constexpr int SAVE_VERSION         = 97;
constexpr int MINOR_VERSION        = 0;
constexpr int BUILD_NUM            = 352;
constexpr int SNAPSHOT_ID          = @SNAPSHOT_ID@;
constexpr int MOD_ID               = @MOD_ID@;
constexpr int FUTURE_SAVE_VERSION  = 97;
constexpr int FUTURE_MINOR_VERSION = 0;
//VersionInfoEnd

#if defined(SNAPSHOT)
constexpr char IDENT_RELTYPE[] = "S";
#elif defined(BETA)
constexpr char IDENT_RELTYPE[] = "B";
#else
constexpr char IDENT_RELTYPE[] = "R";
#endif

constexpr char SCHEME[]         = "https://";
constexpr char STATICSCHEME[]   = "https://";
constexpr char LOCAL_SAVE_DIR[] = "Saves";
constexpr char STAMPS_DIR[]     = "stamps";
constexpr char BRUSH_DIR[]      = "Brushes";

constexpr float M_GRAV = 6.67300e-1f;

#ifdef RENDERER
constexpr int MENUSIZE = 0;
constexpr int BARSIZE  = 0;
#else
constexpr int MENUSIZE = 40;
constexpr int BARSIZE  = 17;
#endif
//CELL, the size of the pressure, gravity, and wall maps. Larger than 1 to prevent extreme lag
constexpr int CELL   =   4;
constexpr int XCELLS = 153;
constexpr int YCELLS =  96;
constexpr int NCELL  = XCELLS * YCELLS;
constexpr int XRES   = XCELLS * CELL;
constexpr int YRES   = YCELLS * CELL;
constexpr int NPART  = XRES * YRES;

constexpr int XCNTR = XRES / 2;
constexpr int YCNTR = YRES / 2;

constexpr int WINDOWW = XRES + BARSIZE;
constexpr int WINDOWH = YRES + MENUSIZE;

constexpr int MAXSIGNS = 16;

constexpr int   ISTP            = CELL / 2;
constexpr float CFDS            = 4.0f / CELL;
constexpr float SIM_MAXVELOCITY = 1e4f;

//Air constants
constexpr float AIR_TSTEPP = 0.3f;
constexpr float AIR_TSTEPV = 0.4f;
constexpr float AIR_VADV   = 0.3f;
constexpr float AIR_VLOSS  = 0.999f;
constexpr float AIR_PLOSS  = 0.9999f;

constexpr int NGOL = 24;

constexpr int CIRCLE_BRUSH = 0;
constexpr int SQUARE_BRUSH = 1;
constexpr int TRI_BRUSH    = 2;
constexpr int BRUSH_NUM    = 3;

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
