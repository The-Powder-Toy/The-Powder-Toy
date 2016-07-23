#include <vector>

#define SC_WALL 0
#define SC_ELEC 1
#define SC_POWERED 2
#define SC_SENSOR 3
#define SC_FORCE 4
#define SC_EXPLOSIVE 5
#define SC_GAS 6
#define SC_LIQUID 7
#define SC_POWDERS 8
#define SC_SOLIDS 9
#define SC_NUCLEAR 10
#define SC_SPECIAL 11
#define SC_LIFE 12
#define SC_TOOL 13
#define SC_FAVORITES 14
#define SC_DECO 15
#define SC_CRACKER 16
#define SC_CRACKER2 17
#define SC_TOTAL 16

#define O_WL_WALLELEC	122
#define O_WL_EWALL		123
#define O_WL_DETECT		124
#define O_WL_STREAM		125
#define O_WL_SIGN		126
#define O_WL_FAN		127
#define O_WL_FANHELPER	255
#define O_WL_ALLOWLIQUID	128
#define O_WL_DESTROYALL	129
#define O_WL_ERASE		130
#define O_WL_WALL		131
#define O_WL_ALLOWAIR	132
#define O_WL_ALLOWSOLID	133
#define O_WL_ALLOWALLELEC	134
#define O_WL_EHOLE		135
#define O_WL_ALLOWGAS	140
#define O_WL_GRAV		142
#define O_WL_ALLOWENERGY 145

#define WL_ERASE		0
#define WL_WALLELEC		1
#define WL_EWALL		2
#define WL_DETECT		3
#define WL_STREAM		4
#define WL_FAN			5
#define WL_ALLOWLIQUID	6
#define WL_DESTROYALL	7
#define WL_WALL			8
#define WL_ALLOWAIR		9
#define WL_ALLOWPOWDER	10
#define WL_ALLOWALLELEC	11
#define WL_EHOLE		12
#define WL_ALLOWGAS		13
#define WL_GRAV			14
#define WL_ALLOWENERGY	15
#define WL_BLOCKAIR		16
#define WL_ERASEALL		17
#define WL_FLOODHELPER	255

#define UI_WALLCOUNT 18

#define OLD_SPC_AIR 236
#define SPC_AIR 256

#define DECO_DRAW	0
#define DECO_CLEAR	1
#define DECO_ADD	2
#define DECO_SUBTRACT	3
#define DECO_MULTIPLY	4
#define DECO_DIVIDE	5
#define DECO_SMUDGE 6

//Old IDs for GOL types
#define GT_GOL 78
#define GT_HLIF 79
#define GT_ASIM 80
#define GT_2x2 81
#define GT_DANI 82
#define GT_AMOE 83
#define GT_MOVE 84
#define GT_PGOL 85
#define GT_DMOE 86
#define GT_34 87
#define GT_LLIF 88
#define GT_STAN 89
#define GT_SEED 134
#define GT_MAZE 135
#define GT_COAG 136
#define GT_WALL 137
#define GT_GNAR 138
#define GT_REPL 139
#define GT_MYST 140
#define GT_LOTE 142
#define GT_FRG2 143
#define GT_STAR 144
#define GT_FROG 145
#define GT_BRAN 146

//New IDs for GOL types
#define NGT_GOL 0
#define NGT_HLIF 1
#define NGT_ASIM 2
#define NGT_2x2 3
#define NGT_DANI 4
#define NGT_AMOE 5
#define NGT_MOVE 6
#define NGT_PGOL 7
#define NGT_DMOE 8
#define NGT_34 9
#define NGT_LLIF 10
#define NGT_STAN 11
#define NGT_SEED 12
#define NGT_MAZE 13
#define NGT_COAG 14
#define NGT_WALL 15
#define NGT_GNAR 16
#define NGT_REPL 17
#define NGT_MYST 18
#define NGT_LOTE 19
#define NGT_FRG2 20
#define NGT_STAR 21
#define NGT_FROG 22
#define NGT_BRAN 23

//replace mode / specific delete flags
#define REPLACE_MODE 0x1
#define SPECIFIC_DELETE 0x2

#ifndef SIMULATIONDATA_H
#define SIMULATIONDATA_H

struct part_type;
struct part_transition;

struct wall_type;
struct gol_menu;
struct menu_section;

class SimTool;
class Element;

gol_menu * LoadGOLMenu(int & golMenuCount);

int * LoadGOLTypes(int & golTypeCount);

int * LoadGOLRules(int & golRuleCount);

wall_type * LoadWalls(int & wallCount);

menu_section * LoadMenus(int & menuCount);

unsigned int * LoadLatent(int & elementCount);

#endif /* SIMULATIONDATA_H */
