#pragma once
#include "SimulationConfig.h"
#include "ElementDefs.h"
#include "common/ExplicitSingleton.h"
#include "common/String.h"
#include "MenuSection.h"
#include "BuiltinGOL.h"
#include "SimTool.h"
#include "Element.h"
#include "Particle.h"
#include "WallType.h"
#include "graphics/gcache_item.h"
#include <cstdint>
#include <vector>
#include <array>
#include <shared_mutex>

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
	EDGE_VOID, EDGE_SOLID, EDGE_LOOP, NUM_EDGEMODES
};

enum AirMode
{
	AIR_ON, AIR_PRESSUREOFF, AIR_VELOCITYOFF, AIR_OFF, AIR_NOUPDATE, NUM_AIRMODES
};

enum GravityMode
{
	GRAV_VERTICAL, GRAV_OFF, GRAV_RADIAL, GRAV_CUSTOM, NUM_GRAVMODES
};

enum DecoSpace
{
	DECOSPACE_SRGB,
	DECOSPACE_LINEAR,
	DECOSPACE_GAMMA22,
	DECOSPACE_GAMMA18,
	NUM_DECOSPACES,
};

struct CustomGOLData
{
	int rule, colour1, colour2;
	String nameString, ruleString;

	inline bool operator <(const CustomGOLData &other) const
	{
		return rule < other.rule;
	}
};

class SimulationData : public ExplicitSingleton<SimulationData>
{
public:
	std::array<Element, PT_NUM> elements;
	std::array<gcache_item, PT_NUM> graphicscache;
	std::vector<SimTool> tools;
	std::vector<wall_type> wtypes;
	std::vector<menu_section> msections;
	char can_move[PT_NUM][PT_NUM];
	static const std::array<BuiltinGOL, NGOL> builtinGol;

	// Element properties that enable basic graphics (i.e. every property that has to do with graphics other than
	// the graphics callback itself) are only ever written by the main thread, but they are read by some other
	// threads that use Renderer to render thumbnails and such. Take this std::shared_mutex with an std::unique_lock
	// when writing such properties in the main thread, and with an std::shared_lock when reading such properties
	// in these secondary Renderer threads. Don't take it with an std::shared_lock when reading such properties in
	// the main thread; the main thread doesn't race with itself.
	mutable std::shared_mutex elementGraphicsMx;

private:
	std::vector<CustomGOLData> customGol;

public:
	SimulationData();
	void InitElements();

	void init_can_move();

	const CustomGOLData *GetCustomGOLByRule(int rule) const;
	const std::vector<CustomGOLData> &GetCustomGol() const { return customGol; }
	void SetCustomGOL(std::vector<CustomGOLData> newCustomGol);

	String ElementResolve(int type, int ctype) const;
	String BasicParticleInfo(Particle const &sample_part) const;
	int GetParticleType(ByteString type) const;

	bool IsElement(int type) const
	{
		return (type > 0 && type < PT_NUM && elements[type].Enabled);
	}

	bool IsElementOrNone(int type) const
	{
		return (type >= 0 && type < PT_NUM && elements[type].Enabled);
	}

};
