#ifndef POWDER_H
#define POWDER_H

#include "air.h"
#include "graphics.h"
#include "defines.h"
#include "interface.h"
#include "misc.h"

#define CM_COUNT 11
#define CM_CRACK 10
#define CM_LIFE 9
#define CM_GRAD 8
#define CM_NOTHING 7
#define CM_FANCY 6
#define CM_HEAT 5
#define CM_BLOB 4
#define CM_FIRE 3
#define CM_PERS 2
#define CM_PRESS 1
#define CM_VEL 0

#define BRUSH_REPLACEMODE 0x1
#define BRUSH_SPECIFIC_DELETE 0x2

#define UI_WALLSTART 222
#define UI_ACTUALSTART 122
#define UI_WALLCOUNT 25

#define WL_WALLELEC	122
#define WL_EWALL	123
#define WL_DETECT	124
#define WL_STREAM	125
#define WL_SIGN	126
#define WL_FAN	127
#define WL_FANHELPER 255
#define WL_ALLOWLIQUID	128
#define WL_DESTROYALL	129
#define WL_ERASE	130
#define WL_WALL	131
#define WL_ALLOWAIR	132
#define WL_ALLOWSOLID	133
#define WL_ALLOWALLELEC	134
#define WL_EHOLE	135

#define SPC_AIR 236
#define SPC_HEAT 237
#define SPC_COOL 238
#define SPC_VACUUM 239
#define SPC_WIND 241
#define SPC_PGRV 243
#define SPC_NGRV 244
#define SPC_PROP 246

#define WL_ALLOWGAS	140
#define WL_GRAV		142
#define WL_ALLOWENERGY 145


#define PT_NONE 0
#define PT_DUST 1
#define PT_WATR 2
#define PT_OIL 3
#define PT_FIRE 4
#define PT_STNE 5
#define PT_LAVA 6
#define PT_GUNP 7
#define PT_NITR 8
#define PT_CLNE 9
#define PT_GAS 10
#define PT_PLEX 11
#define PT_GOO 12
#define PT_ICEI 13
#define PT_METL 14
#define PT_SPRK 15
#define PT_SNOW 16
#define PT_WOOD 17
#define PT_NEUT 18
#define PT_PLUT 19
#define PT_PLNT 20
#define PT_ACID 21
#define PT_VOID 22
#define PT_WTRV 23
#define PT_CNCT 24
#define PT_DSTW 25
#define PT_SALT 26
#define PT_SLTW 27
#define PT_DMND 28
#define PT_BMTL 29
#define PT_BRMT 30
#define PT_PHOT 31
#define PT_URAN 32
#define PT_WAX  33
#define PT_MWAX 34
#define PT_PSCN 35
#define PT_NSCN 36
#define PT_LNTG 37
#define PT_INSL 38
#define PT_BHOL 39
#define PT_WHOL 40
#define PT_RBDM 41
#define PT_LRBD 42
#define PT_NTCT 43
#define PT_SAND 44
#define PT_GLAS 45
#define PT_PTCT 46
#define PT_BGLA 47
#define PT_THDR 48
#define PT_PLSM 49
#define PT_ETRD 50
#define PT_NICE 51
#define PT_NBLE 52
#define PT_BTRY 53
#define PT_LCRY 54
#define PT_STKM 55
#define PT_SWCH 56
#define PT_SMKE 57
#define PT_DESL 58
#define PT_COAL 59
#define PT_LO2  60
#define PT_O2   61
#define PT_INWR 62
#define PT_YEST 63
#define PT_DYST 64
#define PT_THRM 65
#define PT_GLOW 66
#define PT_BRCK 67
#define PT_HFLM 68
#define PT_FIRW 69
#define PT_FUSE 70
#define PT_FSEP 71
#define PT_AMTR 72
#define PT_BCOL 73
#define PT_PCLN 74
#define PT_HSWC 75
#define PT_IRON 76
#define PT_MORT 77
#define PT_LIFE 78
#define PT_DLAY 79
#define PT_CO2	80
#define PT_DRIC	81
#define PT_CBNW 82
#define PT_STOR 83
#define PT_PVOD 84
#define PT_CONV 85
#define PT_CAUS 86

#define PT_LIGH 87
#define PT_TESC 88
#define PT_DEST 89

#define PT_SPNG 90
#define PT_RIME 91
#define PT_FOG 92
#define PT_BCLN 93
#define PT_LOVE 94
#define PT_DEUT 95
#define PT_WARP 96
#define PT_PUMP 97
#define PT_FWRK 98
#define PT_PIPE 99
#define PT_FRZZ 100
#define PT_FRZW 101
#define PT_GRAV 102
#define PT_BIZR 103
#define PT_BIZRG 104
#define PT_BIZRS 105
#define PT_INST 106
#define PT_ISOZ 107
#define PT_ISZS 108
#define PT_PRTI 109
#define PT_PRTO 110
#define PT_PSTE 111
#define PT_PSTS 112
#define PT_ANAR 113
#define PT_VINE 114
#define PT_INVIS 115
#define PT_EQUALVEL 116 //all particles equal their velocities
#define PT_SPAWN2 117
#define PT_SPAWN 118
#define PT_SHLD1 119
#define PT_SHLD2 120
#define PT_SHLD3 121
#define PT_SHLD4 122
#define PT_LOLZ 123
#define PT_WIFI 124
#define PT_FILT 125
#define PT_ARAY 126
#define PT_BRAY 127
#define PT_STKM2 128
#define PT_BOMB 129
#define PT_C5 130
#define PT_SING 131
#define PT_QRTZ 132
#define PT_PQRT 133
#define PT_EMP 134
#define PT_BREC 135
#define PT_ELEC 136
#define PT_ACEL 137
#define PT_DCEL 138
#define PT_BANG	139
#define PT_IGNT 140
#define PT_BOYL 141

#define OLD_PT_WIND 147
#define PT_H2   148
#define PT_SOAP 149
#define PT_NBHL 150
#define PT_NWHL 151
#define PT_MERC 152
#define PT_PBCN 153
#define PT_GPMP 154
#define PT_CLST 155
#define PT_WIRE 156
#define PT_GBMB 157
#define PT_FIGH 158
#define PT_NUM  159

#define R_TEMP 22
#define MAX_TEMP 9999
#define MIN_TEMP 0
#define O_MAX_TEMP 3500
#define O_MIN_TEMP -273

#define ST_NONE 0
#define ST_SOLID 1
#define ST_LIQUID 2
#define ST_GAS 3
/*
   TODO: We should start to implement these.
*/
#define TYPE_PART			0x00001 //1 Powders
#define TYPE_LIQUID			0x00002 //2 Liquids
#define TYPE_SOLID			0x00004 //4 Solids
#define TYPE_GAS			0x00008 //8 Gasses (Includes plasma)
#define TYPE_ENERGY			0x00010 //16 Energy (Thunder, Light, Neutrons etc.)
#define PROP_CONDUCTS		0x00020 //32 Conducts electricity
#define PROP_BLACK			0x00040 //64 Absorbs Photons (not currently implemented or used, a photwl attribute might be better)
#define PROP_NEUTPENETRATE	0x00080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x00100 //256 Absorbs neutrons, reflect is default (not currently implemented or used)
#define PROP_NEUTPASS		0x00200 //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x00400 //1024 Is deadly for stickman (not currently implemented or used)
#define PROP_HOT_GLOW		0x00800 //2048 Hot Metal Glow
#define PROP_LIFE			0x01000 //4096 Is a GoL type
#define PROP_RADIOACTIVE	0x02000 //8192 Radioactive
#define PROP_LIFE_DEC		0x04000 //2^14 Life decreases by one every frame if > zero
#define PROP_LIFE_KILL		0x08000 //2^15 Kill when life value is <= zero
#define PROP_LIFE_KILL_DEC	0x10000 //2^16 Kill when life value is decremented to <= zero
#define PROP_SPARKSETTLE	0x20000	//2^17 Allow Sparks/Embers to settle

#define FLAG_STAGNANT	1

#define GRAPHICS_FUNC_ARGS particle *cpart, int nx, int ny, int *pixel_mode, int* cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb
#define GRAPHICS_FUNC_SUBCALL_ARGS cpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb


struct particle
{
	int type;
	int life, ctype;
	float x, y, vx, vy;
	float temp;
	float pavg[2];
	int flags;
	int tmp;
	int tmp2;
	unsigned int dcolour;
};
typedef struct particle particle;

int graphics_FIRE(GRAPHICS_FUNC_ARGS);
int graphics_SMKE(GRAPHICS_FUNC_ARGS);
int graphics_PLSM(GRAPHICS_FUNC_ARGS);
int graphics_DEUT(GRAPHICS_FUNC_ARGS);
int graphics_PHOT(GRAPHICS_FUNC_ARGS);
int graphics_NEUT(GRAPHICS_FUNC_ARGS);
int graphics_LAVA(GRAPHICS_FUNC_ARGS);
int graphics_SPRK(GRAPHICS_FUNC_ARGS);
int graphics_QRTZ(GRAPHICS_FUNC_ARGS);
int graphics_CLST(GRAPHICS_FUNC_ARGS);
int graphics_CBNW(GRAPHICS_FUNC_ARGS);
int graphics_SPNG(GRAPHICS_FUNC_ARGS);
int graphics_LIFE(GRAPHICS_FUNC_ARGS);
int graphics_DUST(GRAPHICS_FUNC_ARGS);
int graphics_GRAV(GRAPHICS_FUNC_ARGS);
int graphics_WIFI(GRAPHICS_FUNC_ARGS);
int graphics_PRTI(GRAPHICS_FUNC_ARGS);
int graphics_PRTO(GRAPHICS_FUNC_ARGS);
int graphics_BIZR(GRAPHICS_FUNC_ARGS);
int graphics_PIPE(GRAPHICS_FUNC_ARGS);
int graphics_INVS(GRAPHICS_FUNC_ARGS);
int graphics_ACID(GRAPHICS_FUNC_ARGS);
int graphics_FILT(GRAPHICS_FUNC_ARGS);
int graphics_BRAY(GRAPHICS_FUNC_ARGS);
int graphics_SWCH(GRAPHICS_FUNC_ARGS);
int graphics_THDR(GRAPHICS_FUNC_ARGS);
int graphics_GLOW(GRAPHICS_FUNC_ARGS);
int graphics_LCRY(GRAPHICS_FUNC_ARGS);
int graphics_PCLN(GRAPHICS_FUNC_ARGS);
int graphics_PBCN(GRAPHICS_FUNC_ARGS);
int graphics_DLAY(GRAPHICS_FUNC_ARGS);
int graphics_HSWC(GRAPHICS_FUNC_ARGS);
int graphics_PVOD(GRAPHICS_FUNC_ARGS);
int graphics_STOR(GRAPHICS_FUNC_ARGS);
int graphics_PUMP(GRAPHICS_FUNC_ARGS);
int graphics_GPMP(GRAPHICS_FUNC_ARGS);
int graphics_HFLM(GRAPHICS_FUNC_ARGS);
int graphics_FIRW(GRAPHICS_FUNC_ARGS);
int graphics_BOMB(GRAPHICS_FUNC_ARGS);
int graphics_GBMB(GRAPHICS_FUNC_ARGS);
int graphics_COAL(GRAPHICS_FUNC_ARGS);
int graphics_STKM(GRAPHICS_FUNC_ARGS);
int graphics_STKM2(GRAPHICS_FUNC_ARGS);
int graphics_DEST(GRAPHICS_FUNC_ARGS);
int graphics_EMP(GRAPHICS_FUNC_ARGS);
int graphics_LIGH(GRAPHICS_FUNC_ARGS);
int graphics_FIGH(GRAPHICS_FUNC_ARGS);
int graphics_ELEC(GRAPHICS_FUNC_ARGS);
int graphics_WIRE(GRAPHICS_FUNC_ARGS);
int graphics_ACEL(GRAPHICS_FUNC_ARGS);
int graphics_DCEL(GRAPHICS_FUNC_ARGS);

#define UPDATE_FUNC_ARGS int i, int x, int y, int surround_space, int nt
// to call another update function with same arguments:
#define UPDATE_FUNC_SUBCALL_ARGS i, x, y, surround_space, nt

struct playerst
{
	char comm;           //command cell
	char pcomm;          //previous command
	int elem;            //element power
	float legs[16];      //legs' positions
	float accs[8];       //accelerations
	char spwn;           //if stick man was spawned
	unsigned int frames; //frames since last particle spawn - used when spawning LIGH
};
typedef struct playerst playerst;

int update_ACID(UPDATE_FUNC_ARGS);
int update_ANAR(UPDATE_FUNC_ARGS);
int update_AMTR(UPDATE_FUNC_ARGS);
int update_ARAY(UPDATE_FUNC_ARGS);
int update_BCLN(UPDATE_FUNC_ARGS);
int update_BCOL(UPDATE_FUNC_ARGS);
int update_BMTL(UPDATE_FUNC_ARGS);
int update_BRMT(UPDATE_FUNC_ARGS);
int update_BOMB(UPDATE_FUNC_ARGS);
int update_BOYL(UPDATE_FUNC_ARGS);
int update_BTRY(UPDATE_FUNC_ARGS);
int update_C5(UPDATE_FUNC_ARGS);
int update_CLNE(UPDATE_FUNC_ARGS);
int update_COAL(UPDATE_FUNC_ARGS);
int update_DEUT(UPDATE_FUNC_ARGS);
int update_DSTW(UPDATE_FUNC_ARGS);
int update_FOG(UPDATE_FUNC_ARGS);
int update_FRZW(UPDATE_FUNC_ARGS);
int update_FRZZ(UPDATE_FUNC_ARGS);
int update_FSEP(UPDATE_FUNC_ARGS);
int update_FUSE(UPDATE_FUNC_ARGS);
int update_FIRW(UPDATE_FUNC_ARGS);
int update_FWRK(UPDATE_FUNC_ARGS);
int update_GLAS(UPDATE_FUNC_ARGS);
int update_GLOW(UPDATE_FUNC_ARGS);
int update_GOO(UPDATE_FUNC_ARGS);
int update_HSWC(UPDATE_FUNC_ARGS);
int update_IRON(UPDATE_FUNC_ARGS);
int update_ICEI(UPDATE_FUNC_ARGS);
int update_ISZ(UPDATE_FUNC_ARGS);
int update_LCRY(UPDATE_FUNC_ARGS);
int update_MORT(UPDATE_FUNC_ARGS);
int update_NEUT(UPDATE_FUNC_ARGS);
int update_NPTCT(UPDATE_FUNC_ARGS);
int update_PCLN(UPDATE_FUNC_ARGS);
int update_PHOT(UPDATE_FUNC_ARGS);
int update_PIPE(UPDATE_FUNC_ARGS);
int update_PLNT(UPDATE_FUNC_ARGS);
int update_PLUT(UPDATE_FUNC_ARGS);
int update_PRTI(UPDATE_FUNC_ARGS);
int update_PRTO(UPDATE_FUNC_ARGS);
int update_PYRO(UPDATE_FUNC_ARGS);
int update_PUMP(UPDATE_FUNC_ARGS);
int update_QRTZ(UPDATE_FUNC_ARGS);
int update_RIME(UPDATE_FUNC_ARGS);
int update_SHLD1(UPDATE_FUNC_ARGS);
int update_SHLD2(UPDATE_FUNC_ARGS);
int update_SHLD3(UPDATE_FUNC_ARGS);
int update_SHLD4(UPDATE_FUNC_ARGS);
int update_SING(UPDATE_FUNC_ARGS);
int update_SLTW(UPDATE_FUNC_ARGS);
int update_SPAWN(UPDATE_FUNC_ARGS);
int update_SPAWN2(UPDATE_FUNC_ARGS);
int update_SPNG(UPDATE_FUNC_ARGS);
int update_SPRK(UPDATE_FUNC_ARGS);
int update_STKM(UPDATE_FUNC_ARGS);
int update_STKM2(UPDATE_FUNC_ARGS);
int update_SWCH(UPDATE_FUNC_ARGS);
int update_THDR(UPDATE_FUNC_ARGS);
int update_THRM(UPDATE_FUNC_ARGS);
int update_URAN(UPDATE_FUNC_ARGS);
int update_VINE(UPDATE_FUNC_ARGS);
int update_WARP(UPDATE_FUNC_ARGS);
int update_WATR(UPDATE_FUNC_ARGS);
int update_WIFI(UPDATE_FUNC_ARGS);
int update_WTRV(UPDATE_FUNC_ARGS);
int update_YEST(UPDATE_FUNC_ARGS);
int update_SOAP(UPDATE_FUNC_ARGS);
int update_O2(UPDATE_FUNC_ARGS);
int update_H2(UPDATE_FUNC_ARGS);
int update_NBHL(UPDATE_FUNC_ARGS);
int update_NWHL(UPDATE_FUNC_ARGS);
int update_MERC(UPDATE_FUNC_ARGS);
int update_PBCN(UPDATE_FUNC_ARGS);
int update_GPMP(UPDATE_FUNC_ARGS);
int update_CLST(UPDATE_FUNC_ARGS);
int update_DLAY(UPDATE_FUNC_ARGS);
int update_WIRE(UPDATE_FUNC_ARGS);
int update_GBMB(UPDATE_FUNC_ARGS);
int update_CO2(UPDATE_FUNC_ARGS);
int update_CBNW(UPDATE_FUNC_ARGS);
int update_STOR(UPDATE_FUNC_ARGS);
int update_BIZR(UPDATE_FUNC_ARGS);
int update_PVOD(UPDATE_FUNC_ARGS);
int update_CONV(UPDATE_FUNC_ARGS);
int update_CAUS(UPDATE_FUNC_ARGS);
int update_DEST(UPDATE_FUNC_ARGS);
int update_EMP(UPDATE_FUNC_ARGS);
int update_LIGH(UPDATE_FUNC_ARGS);
int update_FIGH(UPDATE_FUNC_ARGS);
int update_ELEC(UPDATE_FUNC_ARGS);
int update_ACEL(UPDATE_FUNC_ARGS);
int update_DCEL(UPDATE_FUNC_ARGS);
int update_BANG(UPDATE_FUNC_ARGS);
int update_IGNT(UPDATE_FUNC_ARGS);

int update_MISC(UPDATE_FUNC_ARGS);
int update_legacy_PYRO(UPDATE_FUNC_ARGS);
int update_legacy_all(UPDATE_FUNC_ARGS);
int run_stickman(playerst* playerp, UPDATE_FUNC_ARGS);
void STKM_init_legs(playerst* playerp, int i);
void STKM_interact(playerst* playerp, int i, int x, int y);

struct part_type
{
	char *name;
	pixel pcolors;
	float advection;
	float airdrag;
	float airloss;
	float loss;
	float collision;
	float gravity;
	float diffusion;
	float hotair;
	int falldown;
	int flammable;
	int explosive;
	int meltable;
	int hardness;
	int menu;
	int enabled;
	int weight;
	int menusection;
	float heat;
	unsigned char hconduct;
	char *descs;
	char state;
	unsigned int properties;
	int (*update_func) (UPDATE_FUNC_ARGS);
	int (*graphics_func) (GRAPHICS_FUNC_ARGS);
};
typedef struct part_type part_type;

struct part_transition
{
	float plv; // transition occurs if pv is lower than this
	int plt;
	float phv; // transition occurs if pv is higher than this
	int pht;
	float tlv; // transition occurs if t is lower than this
	int tlt;
	float thv; // transition occurs if t is higher than this
	int tht;
};
typedef struct part_transition part_transition;

// TODO: falldown, properties, state - should at least one of these be removed?
extern part_type ptypes[PT_NUM];

// temporarily define abbreviations for impossible p/t values
#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1
// no transition (PT_NONE means kill part)
#define NT -1
// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM
extern part_transition ptransitions[PT_NUM];

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

struct gol_menu
{
	const char *name;
	pixel colour;
	int goltype;
	const char *description;
};
typedef struct gol_menu gol_menu;

static gol_menu gmenu[NGOL] = 
{
	{"GOL",		PIXPACK(0x0CAC00), 0, "Game Of Life B3/S23"},
	{"HLIF",	PIXPACK(0xFF0000), 1, "High Life B36/S23"},
	{"ASIM",	PIXPACK(0x0000FF), 2, "Assimilation B345/S4567"},
	{"2x2",		PIXPACK(0xFFFF00), 3, "2x2 B36/S125"},
	{"DANI",	PIXPACK(0x00FFFF), 4, "Day and Night B3678/S34678"},
	{"AMOE",	PIXPACK(0xFF00FF), 5, "Amoeba B357/S1358"},
	{"MOVE",	PIXPACK(0xFFFFFF), 6, "'Move' particles. Does not move things.. it is a life type B368/S245"},
	{"PGOL",	PIXPACK(0xE05010), 7, "Pseudo Life B357/S238"},
	{"DMOE",	PIXPACK(0x500000), 8, "Diamoeba B35678/S5678"},
	{"34",		PIXPACK(0x500050), 9, "34 B34/S34"},
	{"LLIF",	PIXPACK(0x505050), 10, "Long Life B345/S5"},
	{"STAN",	PIXPACK(0x5000FF), 11, "Stains B3678/S235678"},
	{"SEED",	PIXPACK(0xFBEC7D), 12, "B2/S"},
	{"MAZE",	PIXPACK(0xA8E4A0), 13, "B3/S12345"},
	{"COAG",	PIXPACK(0x9ACD32), 14, "B378/S235678"},
	{"WALL",	PIXPACK(0x0047AB), 15, "B45678/S2345"},
	{"GNAR",	PIXPACK(0xE5B73B), 16, "B1/S1"},
	{"REPL",	PIXPACK(0x259588), 17, "B1357/S1357"},
	{"MYST",	PIXPACK(0x0C3C00), 18, "B3458/S05678"},
	{"LOTE",	PIXPACK(0xFF0000), 19, "Behaves kinda like Living on the Edge S3458/B37/4"},
	{"FRG2",	PIXPACK(0x00FF00), 20, "Like Frogs rule S124/B3/3"},
	{"STAR",	PIXPACK(0x0000FF), 21, "Like Star Wars rule S3456/B278/6"},
	{"FROG",	PIXPACK(0x00AA00), 22, "Frogs S12/B34/3"},
	{"BRAN",	PIXPACK(0xCCCC00), 23, "Brian 6 S6/B246/3"}
};

static int grule[NGOL+1][10] =
{
//	 0,1,2,3,4,5,6,7,8,STATES    live=1  spawn=2 spawn&live=3   States are kind of how long until it dies, normal ones use two states(living,dead) for others the intermediate states live but do nothing
	{0,0,0,0,0,0,0,0,0,2},//blank
	{0,0,1,3,0,0,0,0,0,2},//GOL
	{0,0,1,3,0,0,2,0,0,2},//HLIF
	{0,0,0,2,3,3,1,1,0,2},//ASIM
	{0,1,1,2,0,1,2,0,0,2},//2x2
	{0,0,0,3,1,0,3,3,3,2},//DANI
	{0,1,0,3,0,3,0,2,1,2},//AMOE
	{0,0,1,2,1,1,2,0,2,2},//MOVE
	{0,0,1,3,0,2,0,2,1,2},//PGOL
	{0,0,0,2,0,3,3,3,3,2},//DMOE
	{0,0,0,3,3,0,0,0,0,2},//34
	{0,0,0,2,2,3,0,0,0,2},//LLIF
	{0,0,1,3,0,1,3,3,3,2},//STAN
	{0,0,2,0,0,0,0,0,0,2},//SEED
	{0,1,1,3,1,1,0,0,0,2},//MAZE
	{0,0,1,3,0,1,1,3,3,2},//COAG
	{0,0,1,1,3,3,2,2,2,2},//WALL
	{0,3,0,0,0,0,0,0,0,2},//GNAR
	{0,3,0,3,0,3,0,3,0,2},//REPL
	{1,0,0,2,2,3,1,1,3,2},//MYST
	{0,0,0,3,1,1,0,2,1,4},//LOTE
	{0,1,1,2,1,0,0,0,0,3},//FRG2
	{0,0,2,1,1,1,1,2,2,6},//STAR
	{0,1,1,2,2,0,0,0,0,3},//FROG
	{0,0,2,0,2,0,3,0,0,3},//BRAN
};
static int goltype[NGOL] =
{
	GT_GOL,
	GT_HLIF,
	GT_ASIM,
	GT_2x2,
	GT_DANI,
	GT_AMOE,
	GT_MOVE,
	GT_PGOL,
	GT_DMOE,
	GT_34,
	GT_LLIF,
	GT_STAN,
	GT_SEED,
	GT_MAZE,
	GT_COAG,
	GT_WALL,
	GT_GNAR,
	GT_REPL,
	GT_MYST,
	GT_LOTE,
	GT_FRG2,
	GT_STAR,
	GT_FROG,
	GT_BRAN,
};
static int loverule[9][9] =
{
	{0,0,1,1,0,0,0,0,0},
	{0,1,0,0,1,1,0,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,0,0,0,0,1,0},
	{0,1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0},
	{1,0,0,0,0,0,1,0,0},
	{0,1,0,0,1,1,0,0,0},
	{0,0,1,1,0,0,0,0,0},
};
static int lolzrule[9][9] =
{
	{0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,1,1,0,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{0,1,0,1,1,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
};

struct wall_type
{
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	const char *descs;
};
typedef struct wall_type wall_type;

static wall_type wtypes[] =
{
	{PIXPACK(0xC0C0C0), PIXPACK(0x101010), 0, "Wall. Indestructible. Blocks everything. Conductive."},
	{PIXPACK(0x808080), PIXPACK(0x808080), 0, "E-Wall. Becomes transparent when electricity is connected."},
	{PIXPACK(0xFF8080), PIXPACK(0xFF2008), 1, "Detector. Generates electricity when a particle is inside."},
	{PIXPACK(0x808080), PIXPACK(0x000000), 0, "Streamline. Set start point of a streamline."},
	{PIXPACK(0x808080), PIXPACK(0x000000), 0, "Sign. Click on a sign to edit it or anywhere else to place a new one."},
	{PIXPACK(0x8080FF), PIXPACK(0x000000), 1, "Fan. Accelerates air. Use line tool to set direction and strength."},
	{PIXPACK(0xC0C0C0), PIXPACK(0x101010), 2, "Wall. Blocks most particles but lets liquids through. Conductive."},
	{PIXPACK(0x808080), PIXPACK(0x000000), 1, "Wall. Absorbs particles but lets air currents through."},
	{PIXPACK(0x808080), PIXPACK(0x000000), 0, "Erases walls."},
	{PIXPACK(0x808080), PIXPACK(0x000000), 3, "Wall. Indestructible. Blocks everything."},
	{PIXPACK(0x3C3C3C), PIXPACK(0x000000), 1, "Wall. Indestructible. Blocks particles, allows air"},
	{PIXPACK(0x575757), PIXPACK(0x000000), 1, "Wall. Indestructible. Blocks liquids and gasses, allows solids"},
	{PIXPACK(0xFFFF22), PIXPACK(0x101010), 2, "Conductor, allows particles, conducts electricity"},
	{PIXPACK(0x242424), PIXPACK(0x101010), 0, "E-Hole, absorbs particles, release them when powered"},
	{PIXPACK(0xFFFFFF), PIXPACK(0x000000), -1, "Air, creates airflow and pressure"},
	{PIXPACK(0xFFBB00), PIXPACK(0x000000), -1, "Heats the targetted element."},
	{PIXPACK(0x00BBFF), PIXPACK(0x000000), -1, "Cools the targetted element."},
	{PIXPACK(0x303030), PIXPACK(0x000000), -1, "Vacuum, reduces air pressure."},
	{PIXPACK(0x579777), PIXPACK(0x000000), 1, "Wall. Indestructible. Blocks liquids and solids, allows gasses"},
	{PIXPACK(0x000000), PIXPACK(0x000000), -1, "Drag tool"},
	{PIXPACK(0xFFEE00), PIXPACK(0xAA9900), 4, "Gravity wall"},
	{PIXPACK(0x0000BB), PIXPACK(0x000000), -1, "Postive gravity tool."},
	{PIXPACK(0x000099), PIXPACK(0x000000), -1, "Negative gravity tool."},
	{PIXPACK(0xFFAA00), PIXPACK(0xAA5500), 4, "Energy wall, allows only energy type particles to pass"},
	{PIXPACK(0xFFAA00), PIXPACK(0xAA5500), -1, "Property edit tool"},
};

#define CHANNELS ((int)(MAX_TEMP-73)/100+2)
particle portalp[CHANNELS][8][80];
const particle emptyparticle;
int wireless[CHANNELS][2];
extern int portal_rx[8];
extern int portal_ry[8];

extern int wire_placed;

extern playerst player;
extern playerst player2;

extern playerst fighters[256];
extern unsigned char fighcount;

extern int airMode;

extern particle *parts;
extern particle *cb_parts;
extern int parts_lastActiveIndex;

extern unsigned char bmap[YRES/CELL][XRES/CELL];
extern unsigned char emap[YRES/CELL][XRES/CELL];

extern unsigned char cb_bmap[YRES/CELL][XRES/CELL];
extern unsigned char cb_emap[YRES/CELL][XRES/CELL];

extern int pfree;

extern unsigned pmap[YRES][XRES];
unsigned cb_pmap[YRES][XRES];

unsigned photons[YRES][XRES];

int do_move(int i, int x, int y, float nxf, float nyf);
int try_move(int i, int x, int y, int nx, int ny);
int eval_move(int pt, int nx, int ny, unsigned *rr);
void init_can_move();

static void create_cherenkov_photon(int pp);
static void create_gain_photon(int pp);

void kill_part(int i);

int flood_prop(int x, int y, size_t propoffset, void * propvalue, int proptype);

void detach(int i);

void part_change_type(int i, int x, int y, int t);

int InCurrentBrush(int i, int j, int rx, int ry);

int get_brush_flags();

int create_part(int p, int x, int y, int t);

void delete_part(int x, int y, int flags);

int is_wire(int x, int y);

int is_wire_off(int x, int y);

void set_emap(int x, int y);

int parts_avg(int ci, int ni, int t);

void create_arc(int sx, int sy, int dx, int dy, int midpoints, int variance, int type, int flags);

int nearest_part(int ci, int t, int max_d);

void update_particles_i(pixel *vid, int start, int inc);

void update_particles(pixel *vid);

void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert);

void clear_area(int area_x, int area_y, int area_w, int area_h);

void create_box(int x1, int y1, int x2, int y2, int c, int flags);

int flood_parts(int x, int y, int c, int cm, int bm, int flags);

int create_parts(int x, int y, int rx, int ry, int c, int flags);

void create_line(int x1, int y1, int x2, int y2, int rx, int ry, int c, int flags);

void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate);

void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[]);

void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[]);

#endif
