#pragma once
#include <cstdint>

constexpr bool SET_WINDOW_ICON          = @SET_WINDOW_ICON@;
constexpr bool DEBUG                    = @DEBUG@;
constexpr bool X86                      = @X86@;
constexpr bool BETA                     = @BETA@;
constexpr bool SNAPSHOT                 = @SNAPSHOT@;
constexpr bool MOD                      = @MOD@;
constexpr bool GRAVFFT                  = @GRAVFFT@;
constexpr bool NOHTTP                   = @NOHTTP@;
constexpr bool LUACONSOLE               = @LUACONSOLE@;
constexpr bool ALLOW_FAKE_NEWER_VERSION = @ALLOW_FAKE_NEWER_VERSION@;
constexpr bool USE_UPDATESERVER         = @USE_UPDATESERVER@;
constexpr bool INSTALL_CHECK            = @INSTALL_CHECK@;
constexpr bool IGNORE_UPDATES           = @IGNORE_UPDATES@;
constexpr bool ENFORCE_HTTPS            = @ENFORCE_HTTPS@;
constexpr char PATH_SEP_CHAR            = '@PATH_SEP_CHAR@';

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

constexpr int SAVE_VERSION         = 97;
constexpr int MINOR_VERSION        = 0;
constexpr int BUILD_NUM            = 352;
constexpr int SNAPSHOT_ID          = @SNAPSHOT_ID@;
constexpr int MOD_ID               = @MOD_ID@;
constexpr int FUTURE_SAVE_VERSION  = 97;
constexpr int FUTURE_MINOR_VERSION = 0;

constexpr char IDENT_RELTYPE    = SNAPSHOT ? 'S' : (BETA ? 'B' : 'R');

constexpr char SCHEME[]         = "https://";
constexpr char STATICSCHEME[]   = "https://";
constexpr char LOCAL_SAVE_DIR[] = "Saves";
constexpr char STAMPS_DIR[]     = "stamps";
constexpr char BRUSH_DIR[]      = "Brushes";

constexpr float M_GRAV = 6.67300e-1f;

constexpr int MENUSIZE = 40;
constexpr int BARSIZE  = 17;

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
