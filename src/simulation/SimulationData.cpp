#include "SimulationData.h"
#include "ElementGraphics.h"
#include "ElementDefs.h"
#include "ElementClasses.h"
#include "GOLString.h"
#include "BuiltinGOL.h"
#include "WallType.h"
#include "MenuSection.h"
#include "ToolClasses.h"
#include "Misc.h"
#include "graphics/Renderer.h"

const std::array<BuiltinGOL, NGOL> SimulationData::builtinGol = {{
	// * Ruleset:
	//   * bits x = 8..0: stay if x neighbours present
	//   * bits x = 16..9: begin if x-8 neighbours present
	//   * bits 20..17: 4-bit unsigned int encoding the number of states minus 2; 2 states is
	//     encoded as 0, 3 states as 1, etc.
	//   * states are kind of long until a cell dies; normal ones use two states (living and dead),
	//     for others the intermediate states live but do nothing
	//   * the ruleset constants below look 20-bit, but rulesets actually consist of 21
	//     bits of data; bit 20 just happens to not be set for any of the built-in types,
	//     as none of them have 10 or more states
	{ "GOL",  GT_GOL , 0x0080C, 0x0CAC00_rgb, 0x0CAC00_rgb, NGT_GOL,  String("Game Of Life: Begin 3/Stay 23") },
	{ "HLIF", GT_HLIF, 0x0480C, 0xFF0000_rgb, 0xFF0000_rgb, NGT_HLIF, String("High Life: B36/S23") },
	{ "ASIM", GT_ASIM, 0x038F0, 0x0000FF_rgb, 0x0000FF_rgb, NGT_ASIM, String("Assimilation: B345/S4567") },
	{ "2X2",  GT_2x2 , 0x04826, 0xFFFF00_rgb, 0xFFFF00_rgb, NGT_2x2,  String("2X2: B36/S125") },
	{ "DANI", GT_DANI, 0x1C9D8, 0x00FFFF_rgb, 0x00FFFF_rgb, NGT_DANI, String("Day and Night: B3678/S34678") },
	{ "AMOE", GT_AMOE, 0x0A92A, 0xFF00FF_rgb, 0xFF00FF_rgb, NGT_AMOE, String("Amoeba: B357/S1358") },
	{ "MOVE", GT_MOVE, 0x14834, 0xFFFFFF_rgb, 0xFFFFFF_rgb, NGT_MOVE, String("'Move' particles. Does not move things.. it is a life type: B368/S245") },
	{ "PGOL", GT_PGOL, 0x0A90C, 0xE05010_rgb, 0xE05010_rgb, NGT_PGOL, String("Pseudo Life: B357/S238") },
	{ "DMOE", GT_DMOE, 0x1E9E0, 0x500000_rgb, 0x500000_rgb, NGT_DMOE, String("Diamoeba: B35678/S5678") },
	{ "3-4",  GT_34  , 0x01818, 0x500050_rgb, 0x500050_rgb, NGT_34,   String("3-4: B34/S34") },
	{ "LLIF", GT_LLIF, 0x03820, 0x505050_rgb, 0x505050_rgb, NGT_LLIF, String("Long Life: B345/S5") },
	{ "STAN", GT_STAN, 0x1C9EC, 0x5000FF_rgb, 0x5000FF_rgb, NGT_STAN, String("Stains: B3678/S235678") },
	{ "SEED", GT_SEED, 0x00400, 0xFBEC7D_rgb, 0xFBEC7D_rgb, NGT_SEED, String("Seeds: B2/S") },
	{ "MAZE", GT_MAZE, 0x0083E, 0xA8E4A0_rgb, 0xA8E4A0_rgb, NGT_MAZE, String("Maze: B3/S12345") },
	{ "COAG", GT_COAG, 0x189EC, 0x9ACD32_rgb, 0x9ACD32_rgb, NGT_COAG, String("Coagulations: B378/S235678") },
	{ "WALL", GT_WALL, 0x1F03C, 0x0047AB_rgb, 0x0047AB_rgb, NGT_WALL, String("Walled cities: B45678/S2345") },
	{ "GNAR", GT_GNAR, 0x00202, 0xE5B73B_rgb, 0xE5B73B_rgb, NGT_GNAR, String("Gnarl: B1/S1") },
	{ "REPL", GT_REPL, 0x0AAAA, 0x259588_rgb, 0x259588_rgb, NGT_REPL, String("Replicator: B1357/S1357") },
	{ "MYST", GT_MYST, 0x139E1, 0x0C3C00_rgb, 0x0C3C00_rgb, NGT_MYST, String("Mystery: B3458/S05678") },
	{ "LOTE", GT_LOTE, 0x48938, 0xFF0000_rgb, 0xFFFF00_rgb, NGT_LOTE, String("Living on the Edge: B37/S3458/4") },
	{ "FRG2", GT_FRG2, 0x20816, 0x006432_rgb, 0x00FF5A_rgb, NGT_FRG2, String("Like Frogs rule: B3/S124/3") },
	{ "STAR", GT_STAR, 0x98478, 0x000040_rgb, 0x0000E6_rgb, NGT_STAR, String("Like Star Wars rule: B278/S3456/6") },
	{ "FROG", GT_FROG, 0x21806, 0x006400_rgb, 0x00FF00_rgb, NGT_FROG, String("Frogs: B34/S12/3") },
	{ "BRAN", GT_BRAN, 0x25440, 0xFFFF00_rgb, 0x969600_rgb, NGT_BRAN, String("Brian 6: B246/S6/3" )}
}};

static std::vector<wall_type> LoadWalls()
{
	return
	std::vector<wall_type>{
		{0x808080_rgb, 0x000000_rgb, 0, Renderer::WallIcon, String("ERASE"),           "DEFAULT_WL_ERASE",  String("Erases walls.")},
		{0xC0C0C0_rgb, 0x101010_rgb, 0, Renderer::WallIcon, String("CONDUCTIVE WALL"), "DEFAULT_WL_CNDTW",  String("Blocks everything. Conductive.")},
		{0x808080_rgb, 0x808080_rgb, 0, Renderer::WallIcon, String("EWALL"),           "DEFAULT_WL_EWALL",  String("E-Wall. Becomes transparent when electricity is connected.")},
		{0xFF8080_rgb, 0xFF2008_rgb, 1, Renderer::WallIcon, String("DETECTOR"),        "DEFAULT_WL_DTECT",  String("Detector. Generates electricity when a particle is inside.")},
		{0x808080_rgb, 0x000000_rgb, 0, Renderer::WallIcon, String("STREAMLINE"),      "DEFAULT_WL_STRM",   String("Streamline. Creates a line that follows air movement.")},
		{0x8080FF_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("FAN"),             "DEFAULT_WL_FAN",    String("Fan. Accelerates air. Use the line tool to set direction and strength.")},
		{0xC0C0C0_rgb, 0x101010_rgb, 2, Renderer::WallIcon, String("LIQUID WALL"),     "DEFAULT_WL_LIQD",   String("Allows liquids, blocks all other particles. Conductive.")},
		{0x808080_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("ABSORB WALL"),     "DEFAULT_WL_ABSRB",  String("Absorbs particles but lets air currents through.")},
		{0x808080_rgb, 0x000000_rgb, 3, Renderer::WallIcon, String("WALL"),            "DEFAULT_WL_WALL",   String("Basic wall, blocks everything.")},
		{0x3C3C3C_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("AIRONLY WALL"),    "DEFAULT_WL_AIR",    String("Allows air, but blocks all particles.")},
		{0x575757_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("POWDER WALL"),     "DEFAULT_WL_POWDR",  String("Allows powders, blocks all other particles.")},
		{0xFFFF22_rgb, 0x101010_rgb, 2, Renderer::WallIcon, String("CONDUCTOR"),       "DEFAULT_WL_CNDTR",  String("Conductor. Allows all particles to pass through and conducts electricity.")},
		{0x242424_rgb, 0x101010_rgb, 0, Renderer::WallIcon, String("EHOLE"),           "DEFAULT_WL_EHOLE",  String("E-Hole. absorbs particles, releases them when powered.")},
		{0x579777_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("GAS WALL"),        "DEFAULT_WL_GAS",    String("Allows gases, blocks all other particles.")},
		{0xFFEE00_rgb, 0xAA9900_rgb, 4, Renderer::WallIcon, String("GRAVITY WALL"),    "DEFAULT_WL_GRVTY",  String("Gravity wall. Newtonian Gravity has no effect inside a box drawn with this.")},
		{0xFFAA00_rgb, 0xAA5500_rgb, 4, Renderer::WallIcon, String("ENERGY WALL"),     "DEFAULT_WL_ENRGY",  String("Allows energy particles, blocks all other particles.")},
		{0xDCDCDC_rgb, 0x000000_rgb, 1, Renderer::WallIcon, String("AIRBLOCK WALL"),   "DEFAULT_WL_NOAIR",  String("Allows all particles, but blocks air.")},
		{0x808080_rgb, 0x000000_rgb, 0, Renderer::WallIcon, String("ERASEALL"),        "DEFAULT_WL_ERASEA", String("Erases walls, particles, and signs.")},
		{0x800080_rgb, 0x000000_rgb, 0, Renderer::WallIcon, String("STASIS WALL"),     "DEFAULT_WL_STASIS", String("Freezes particles inside the wall in place until powered.")},
	};
}

static std::vector<menu_section> LoadMenus()
{
	return
	std::vector<menu_section>{
		{0xE041, String("Walls"), 0, 1},
		{0xE042, String("Electronics"), 0, 1},
		{0xE056, String("Powered Materials"), 0, 1},
		{0xE019, String("Sensors"), 0, 1},
		{0xE062, String("Force"), 0, 1},
		{0xE043, String("Explosives"), 0, 1},
		{0xE045, String("Gases"), 0, 1},
		{0xE044, String("Liquids"), 0, 1},
		{0xE050, String("Powders"), 0, 1},
		{0xE051, String("Solids"), 0, 1},
		{0xE046, String("Radioactive"), 0, 1},
		{0xE04C, String("Special"), 0, 1},
		{0xE052, String("Game Of Life"), 0, 1},
		{0xE057, String("Tools"), 0, 1},
		{0xE067, String("Favorites"), 0, 1},
		{0xE064, String("Decoration tools"), 0, 1},
	};
}

void SimulationData::init_can_move()
{
	int movingType, destinationType;
	// can_move[moving type][type at destination]
	//  0 = No move/Bounce
	//  1 = Swap
	//  2 = Both particles occupy the same space.
	//  3 = Varies, go run some extra checks

	//particles that don't exist shouldn't move...
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
		can_move[0][destinationType] = 0;

	//initialize everything else to swapping by default
	for (movingType = 1; movingType < PT_NUM; movingType++)
		for (destinationType = 0; destinationType < PT_NUM; destinationType++)
			can_move[movingType][destinationType] = 1;

	//photons go through everything by default
	for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		can_move[PT_PHOT][destinationType] = 2;

	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		for (destinationType = 1; destinationType < PT_NUM; destinationType++)
		{
			//weight check, also prevents particles of same type displacing each other
			if (elements[movingType].Weight <= elements[destinationType].Weight || destinationType == PT_GEL)
				can_move[movingType][destinationType] = 0;

			//other checks for NEUT and energy particles
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTPASS))
				can_move[movingType][destinationType] = 2;
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTABSORB))
				can_move[movingType][destinationType] = 1;
			if (movingType == PT_NEUT && (elements[destinationType].Properties&PROP_NEUTPENETRATE))
				can_move[movingType][destinationType] = 1;
			if (destinationType == PT_NEUT && (elements[movingType].Properties&PROP_NEUTPENETRATE))
				can_move[movingType][destinationType] = 0;
			if ((elements[movingType].Properties&TYPE_ENERGY) && (elements[destinationType].Properties&TYPE_ENERGY))
				can_move[movingType][destinationType] = 2;
		}
	}
	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		//set what stickmen can move through
		int stkm_move = 0;
		if (elements[destinationType].Properties & (TYPE_LIQUID | TYPE_GAS))
			stkm_move = 2;
		if (!destinationType || destinationType == PT_PRTO || destinationType == PT_SPAWN || destinationType == PT_SPAWN2)
			stkm_move = 2;
		can_move[PT_STKM][destinationType] = stkm_move;
		can_move[PT_STKM2][destinationType] = stkm_move;
		can_move[PT_FIGH][destinationType] = stkm_move;

		//spark shouldn't move
		can_move[PT_SPRK][destinationType] = 0;
	}
	for (movingType = 1; movingType < PT_NUM; movingType++)
	{
		//everything "swaps" with VACU and BHOL to make them eat things
		can_move[movingType][PT_BHOL] = 1;
		can_move[movingType][PT_NBHL] = 1;
		//nothing goes through stickmen
		can_move[movingType][PT_STKM] = 0;
		can_move[movingType][PT_STKM2] = 0;
		can_move[movingType][PT_FIGH] = 0;
		//INVS behaviour varies with pressure
		can_move[movingType][PT_INVIS] = 3;
		//stop CNCT from being displaced by other particles
		can_move[movingType][PT_CNCT] = 0;
		//VOID and PVOD behaviour varies with powered state and ctype
		can_move[movingType][PT_PVOD] = 3;
		can_move[movingType][PT_VOID] = 3;
		//nothing moves through EMBR (not sure why, but it's killed when it touches anything)
		can_move[movingType][PT_EMBR] = 0;
		can_move[PT_EMBR][movingType] = 0;
		//Energy particles move through VIBR and BVBR, so it can absorb them
		if (elements[movingType].Properties & TYPE_ENERGY)
		{
			can_move[movingType][PT_VIBR] = 1;
			can_move[movingType][PT_BVBR] = 1;
		}

		//SAWD cannot be displaced by other powders
		if (elements[movingType].Properties & TYPE_PART)
			can_move[movingType][PT_SAWD] = 0;
	}

	for (destinationType = 0; destinationType < PT_NUM; destinationType++)
	{
		//a list of lots of things PHOT can move through
		if (elements[destinationType].Properties & PROP_PHOTPASS)
			can_move[PT_PHOT][destinationType] = 2;

		//Things PROT and GRVT cannot move through
		if (destinationType != PT_DMND && destinationType != PT_INSL && destinationType != PT_VOID && destinationType != PT_PVOD && destinationType != PT_VIBR && destinationType != PT_BVBR && destinationType != PT_PRTI && destinationType != PT_PRTO)
		{
			can_move[PT_PROT][destinationType] = 2;
			can_move[PT_GRVT][destinationType] = 2;
		}
	}

	//other special cases that weren't covered above
	can_move[PT_DEST][PT_DMND] = 0;
	can_move[PT_DEST][PT_CLNE] = 0;
	can_move[PT_DEST][PT_PCLN] = 0;
	can_move[PT_DEST][PT_BCLN] = 0;
	can_move[PT_DEST][PT_PBCN] = 0;
	can_move[PT_DEST][PT_ROCK] = 0;

	can_move[PT_NEUT][PT_INVIS] = 2;
	can_move[PT_ELEC][PT_LCRY] = 2;
	can_move[PT_ELEC][PT_EXOT] = 2;
	can_move[PT_ELEC][PT_GLOW] = 2;
	can_move[PT_PHOT][PT_LCRY] = 3; //varies according to LCRY life
	can_move[PT_PHOT][PT_GPMP] = 3;

	can_move[PT_PHOT][PT_BIZR] = 2;
	can_move[PT_ELEC][PT_BIZR] = 2;
	can_move[PT_PHOT][PT_BIZRG] = 2;
	can_move[PT_ELEC][PT_BIZRG] = 2;
	can_move[PT_PHOT][PT_BIZRS] = 2;
	can_move[PT_ELEC][PT_BIZRS] = 2;
	can_move[PT_BIZR][PT_FILT] = 2;
	can_move[PT_BIZRG][PT_FILT] = 2;

	can_move[PT_ANAR][PT_WHOL] = 1; //WHOL eats ANAR
	can_move[PT_ANAR][PT_NWHL] = 1;
	can_move[PT_ELEC][PT_DEUT] = 1;
	can_move[PT_THDR][PT_THDR] = 2;
	can_move[PT_EMBR][PT_EMBR] = 2;
	can_move[PT_TRON][PT_SWCH] = 3;
	can_move[PT_ELEC][PT_RSST] = 2;
	can_move[PT_ELEC][PT_RSSS] = 2;
}

const CustomGOLData *SimulationData::GetCustomGOLByRule(int rule) const
{
	// * Binary search. customGol is already sorted, see SetCustomGOL.
	auto it = std::lower_bound(customGol.begin(), customGol.end(), rule, [](const CustomGOLData &item, int rule) {
		return item.rule < rule;
	});
	if (it != customGol.end() && !(rule < it->rule))
	{
		return &*it;
	}
	return nullptr;
}

void SimulationData::SetCustomGOL(std::vector<CustomGOLData> newCustomGol)
{
	std::sort(newCustomGol.begin(), newCustomGol.end());
	customGol = newCustomGol;
}

String SimulationData::ElementResolve(int type, int ctype) const
{
	if (type == PT_LIFE)
	{
		if (ctype >= 0 && ctype < NGOL)
		{
			return builtinGol[ctype].name; 
		}
		auto *cgol = GetCustomGOLByRule(ctype);
		if (cgol)
		{
			return cgol->nameString;
		}
		return SerialiseGOLRule(ctype);
	}
	else if (type >= 0 && type < PT_NUM)
		return elements[type].Name;
	return "Empty";
}

String SimulationData::BasicParticleInfo(Particle const &sample_part) const
{
	StringBuilder sampleInfo;
	int type = sample_part.type;
	int ctype = sample_part.ctype;
	int storedCtype = sample_part.tmp4;
	if (type == PT_LAVA && IsElement(ctype))
	{
		sampleInfo << "Molten " << ElementResolve(ctype, -1);
	}
	else if ((type == PT_PIPE || type == PT_PPIP) && IsElement(ctype))
	{
		if (ctype == PT_LAVA && IsElement(storedCtype))
		{
			sampleInfo << ElementResolve(type, -1) << " with molten " << ElementResolve(storedCtype, -1);
		}
		else
		{
			sampleInfo << ElementResolve(type, -1) << " with " << ElementResolve(ctype, storedCtype);
		}
	}
	else
	{
		sampleInfo << ElementResolve(type, ctype);
	}
	return sampleInfo.Build();
}

int SimulationData::GetParticleType(ByteString type) const
{
	type = type.ToUpper();

	// alternative names for some elements
	if (byteStringEqualsLiteral(type, "C4"))
	{
		return PT_PLEX;
	}
	else if (byteStringEqualsLiteral(type, "C5"))
	{
		return PT_C5;
	}
	else if (byteStringEqualsLiteral(type, "NONE"))
	{
		return PT_NONE;
	}
	for (int i = 1; i < PT_NUM; i++)
	{
		if (elements[i].Name.size() && elements[i].Enabled && type == elements[i].Name.ToUtf8().ToUpper())
		{
			return i;
		}
	}
	return -1;
}

SimulationData::SimulationData()
{
	msections = LoadMenus();
	wtypes = LoadWalls();
	elements = GetElements();
	tools = GetTools();
	init_can_move();
}
