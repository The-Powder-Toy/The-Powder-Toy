#pragma once
#include "SimulationConfig.h"
#include <cstdint>
#include <vector>
#include <array>

constexpr int SC_WALL      =  0;
constexpr int SC_ELEC      =  1;
constexpr int SC_POWERED   =  2;
constexpr int SC_SENSOR    =  3;
constexpr int SC_FORCE     =  4;
constexpr int SC_EXPLOSIVE =  5;
constexpr int SC_GAS       =  6;
constexpr int SC_LIQUID    =  7;
constexpr int SC_POWDERS   =  8;
constexpr int SC_SOLIDS    =  9;
constexpr int SC_NUCLEAR   = 10;
constexpr int SC_SPECIAL   = 11;
constexpr int SC_LIFE      = 12;
constexpr int SC_TOOL      = 13;
constexpr int SC_FAVORITES = 14;
constexpr int SC_DECO      = 15;
constexpr int SC_CRACKER   = 16;
constexpr int SC_CRACKER2  = 17;
constexpr int SC_TOTAL     = 16;

constexpr int O_WL_WALLELEC     = 122;
constexpr int O_WL_EWALL        = 123;
constexpr int O_WL_DETECT       = 124;
constexpr int O_WL_STREAM       = 125;
constexpr int O_WL_SIGN         = 126;
constexpr int O_WL_FAN          = 127;
constexpr int O_WL_FANHELPER    = 255;
constexpr int O_WL_ALLOWLIQUID  = 128;
constexpr int O_WL_DESTROYALL   = 129;
constexpr int O_WL_ERASE        = 130;
constexpr int O_WL_WALL         = 131;
constexpr int O_WL_ALLOWAIR     = 132;
constexpr int O_WL_ALLOWSOLID   = 133;
constexpr int O_WL_ALLOWALLELEC = 134;
constexpr int O_WL_EHOLE        = 135;
constexpr int O_WL_ALLOWGAS     = 140;
constexpr int O_WL_GRAV         = 142;
constexpr int O_WL_ALLOWENERGY  = 145;

constexpr int WL_ERASE        =  0;
constexpr int WL_WALLELEC     =  1;
constexpr int WL_EWALL        =  2;
constexpr int WL_DETECT       =  3;
constexpr int WL_STREAM       =  4;
constexpr int WL_FAN          =  5;
constexpr int WL_ALLOWLIQUID  =  6;
constexpr int WL_DESTROYALL   =  7;
constexpr int WL_WALL         =  8;
constexpr int WL_ALLOWAIR     =  9;
constexpr int WL_ALLOWPOWDER  = 10;
constexpr int WL_ALLOWALLELEC = 11;
constexpr int WL_EHOLE        = 12;
constexpr int WL_ALLOWGAS     = 13;
constexpr int WL_GRAV         = 14;
constexpr int WL_ALLOWENERGY  = 15;
constexpr int WL_BLOCKAIR     = 16;
constexpr int WL_ERASEALL     = 17;
constexpr int WL_STASIS       = 18;
constexpr int WL_FLOODHELPER  =255;

constexpr int UI_WALLCOUNT = 19;

constexpr int OLD_SPC_AIR = 236;
constexpr int SPC_AIR     = 256;

constexpr int DECO_DRAW     = 0;
constexpr int DECO_CLEAR    = 1;
constexpr int DECO_ADD      = 2;
constexpr int DECO_SUBTRACT = 3;
constexpr int DECO_MULTIPLY = 4;
constexpr int DECO_DIVIDE   = 5;
constexpr int DECO_SMUDGE   = 6;

//Old IDs for GOL types
constexpr int GT_GOL  =  78;
constexpr int GT_HLIF =  79;
constexpr int GT_ASIM =  80;
constexpr int GT_2x2  =  81;
constexpr int GT_DANI =  82;
constexpr int GT_AMOE =  83;
constexpr int GT_MOVE =  84;
constexpr int GT_PGOL =  85;
constexpr int GT_DMOE =  86;
constexpr int GT_34   =  87;
constexpr int GT_LLIF =  88;
constexpr int GT_STAN =  89;
constexpr int GT_SEED = 134;
constexpr int GT_MAZE = 135;
constexpr int GT_COAG = 136;
constexpr int GT_WALL = 137;
constexpr int GT_GNAR = 138;
constexpr int GT_REPL = 139;
constexpr int GT_MYST = 140;
constexpr int GT_LOTE = 142;
constexpr int GT_FRG2 = 143;
constexpr int GT_STAR = 144;
constexpr int GT_FROG = 145;
constexpr int GT_BRAN = 146;

//New IDs for GOL types
constexpr int NGT_GOL  =  0;
constexpr int NGT_HLIF =  1;
constexpr int NGT_ASIM =  2;
constexpr int NGT_2x2  =  3;
constexpr int NGT_DANI =  4;
constexpr int NGT_AMOE =  5;
constexpr int NGT_MOVE =  6;
constexpr int NGT_PGOL =  7;
constexpr int NGT_DMOE =  8;
constexpr int NGT_34   =  9;
constexpr int NGT_LLIF = 10;
constexpr int NGT_STAN = 11;
constexpr int NGT_SEED = 12;
constexpr int NGT_MAZE = 13;
constexpr int NGT_COAG = 14;
constexpr int NGT_WALL = 15;
constexpr int NGT_GNAR = 16;
constexpr int NGT_REPL = 17;
constexpr int NGT_MYST = 18;
constexpr int NGT_LOTE = 19;
constexpr int NGT_FRG2 = 20;
constexpr int NGT_STAR = 21;
constexpr int NGT_FROG = 22;
constexpr int NGT_BRAN = 23;

//replace mode / specific delete flags
constexpr auto REPLACE_MODE    = UINT32_C(0x00000001);
constexpr auto SPECIFIC_DELETE = UINT32_C(0x00000002);

enum EdgeMode
{
	EDGE_VOID, EDGE_SOLID, EDGE_LOOP, NUM_EDGE_MODES
};

enum AirMode
{
	AIR_ON, AIR_PRESSURE_OFF, AIR_VELOCITY_OFF, AIR_OFF, AIR_NO_UPDATE, NUM_AIR_MODES
};

enum GravityMode
{
	GRAV_VERTICAL, GRAV_OFF, GRAV_RADIAL, GRAV_CUSTOM, NUM_GRAV_MODES
};

struct part_type;
struct part_transition;

struct wall_type;
struct BuiltinGOL;
struct menu_section;

class SimTool;
class Element;

extern const BuiltinGOL builtinGol[];

std::vector<wall_type> LoadWalls();

std::vector<menu_section> LoadMenus();

std::vector<unsigned int> LoadLatent();
