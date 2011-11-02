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
#define TYPE_PART			0x0001 //1 Powders
#define TYPE_LIQUID			0x0002 //2 Liquids
#define TYPE_SOLID			0x0004 //4 Solids
#define TYPE_GAS			0x0008 //8 Gasses (Includes plasma)
#define TYPE_ENERGY			0x0010 //16 Energy (Thunder, Light, Neutrons etc.)
#define PROP_CONDUCTS		0x0020 //32 Conducts electricity
#define PROP_BLACK			0x0040 //64 Absorbs Photons (not currently implemented or used, a photwl attribute might be better)
#define PROP_NEUTPENETRATE	0x0080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x0100 //256 Absorbs neutrons, reflect is default (not currently implemented or used)
#define PROP_NEUTPASS		0x0200 //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x0400 //1024 Is deadly for stickman (not currently implemented or used)
#define PROP_HOT_GLOW		0x0800 //2048 Hot Metal Glow
#define PROP_LIFE			0x1000 //4096 Is a GoL type
#define PROP_RADIOACTIVE	0x2000 //8192 Radioactive
#define PROP_LIFE_DEC		0x4000 //2^14 Life decreases by one every frame if > zero
#define PROP_LIFE_KILL		0x8000 //2^15 Kill when life value is <= zero
#define PROP_LIFE_KILL_DEC	0x10000 //2^16 Kill when life value is decremented to <= zero

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

int update_MISC(UPDATE_FUNC_ARGS);
int update_legacy_PYRO(UPDATE_FUNC_ARGS);
int update_legacy_all(UPDATE_FUNC_ARGS);
int run_stickman(playerst* playerp, UPDATE_FUNC_ARGS);
void STKM_init_legs(playerst* playerp, int i);
void STKM_interact(playerst* playerp, int i, int x, int y);

struct part_type
{
	const char *name;
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
	const char *descs;
	char state;
	const unsigned int properties;
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


//Should probably be const.
/* Weight Help
 * 1   = Gas   ||
 * 2   = Light || Liquids  0-49
 * 98  = Heavy || Powder  50-99
 * 100 = Solid ||
 * -1 is Neutrons and Photons
 */
// TODO: falldown, properties, state - should at least one of these be removed?
static const part_type ptypes[PT_NUM] =
{
	//Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd M	Use	Weight	Section			H						Ins		Description
	{"",		PIXPACK(0x000000),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Erases particles.", ST_NONE, 0, NULL, NULL},
	{"DUST",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Flammable.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, NULL, &graphics_DUST},
	{"WATR",	PIXPACK(0x2030D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Liquid. Conducts electricity. Freezes. Extinguishes fires.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_WATR, NULL},
	{"OIL",		PIXPACK(0x404010),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	20,		0,	0,	5,	1,	1,	20,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Flammable.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"FIRE",	PIXPACK(0xFF1000),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	2,		SC_EXPLOSIVE,	R_TEMP+400.0f+273.15f,	88,		"Ignites flammable materials. Heats air.", ST_GAS, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_PYRO, &graphics_FIRE},
	{"STNE",	PIXPACK(0xA0A0A0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Heavy particles. Meltable.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"LAVA",	PIXPACK(0xE05010),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.0003f	* CFDS,	2,	0,		0,	0,	2,	1,	1,	45,		SC_LIQUID,		R_TEMP+1500.0f+273.15f,	60,		"Heavy liquid. Ignites flammable materials. Solidifies when cold.", ST_LIQUID, TYPE_LIQUID|PROP_LIFE_DEC, &update_PYRO, &graphics_LAVA},
	{"GUN",		PIXPACK(0xC0C0D0),	0.7f,	0.02f * CFDS,	0.94f,	0.80f,	-0.1f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	600,	1,	0,	10,	1,	1,	85,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	97,		"Light dust. Explosive.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"NITR",	PIXPACK(0x20E010),	0.5f,	0.02f * CFDS,	0.92f,	0.97f,	0.0f,	0.2f,	0.00f,	0.000f	* CFDS,	2,	1000,	2,	0,	3,	1,	1,	23,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	50,		"Liquid. Pressure sensitive explosive.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"CLNE",	PIXPACK(0xFFD010),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Duplicates any particles it touches.", ST_SOLID, TYPE_SOLID, &update_CLNE, NULL},
	{"GAS",		PIXPACK(0xE0FF20),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	600,	0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	42,		"Gas. Diffuses. Flammable. Liquefies under pressure.", ST_GAS, TYPE_GAS, NULL, NULL},
	{"C-4",		PIXPACK(0xD080E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	2,	50,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Solid. Pressure sensitive explosive.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, NULL, NULL},
	{"GOO",		PIXPACK(0x804000),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	75,		"Solid. Deforms and disappears under pressure.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_GOO, NULL},
	{"ICE",		PIXPACK(0xA0C0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP-50.0f+273.15f,	46,		"Solid. Freezes water. Crushes under pressure. Cools down air.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_ICEI, NULL},
	{"METL",	PIXPACK(0x404060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Conducts electricity. Meltable.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"SPRK",	PIXPACK(0xFFFF80),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electricity. Conducted by metal and water.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SPRK, &graphics_SPRK},
	{"SNOW",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	1,	50,		SC_POWDERS,		R_TEMP-30.0f+273.15f,	46,		"Light particles.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_ICEI, NULL},
	{"WOOD",	PIXPACK(0xC0A040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	15,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Solid. Flammable.", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, NULL, NULL},
	{"NEUT",	PIXPACK(0x20E0FF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	60,		"Neutrons. Interact with matter in odd ways.", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_NEUT, &graphics_NEUT},
	{"PLUT",	PIXPACK(0x407020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	251,	"Heavy particles. Fissile. Generates neutrons under pressure.", ST_SOLID, TYPE_PART|PROP_NEUTPENETRATE|PROP_RADIOACTIVE, &update_PLUT, NULL},
	{"PLNT",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	65,		"Plant, drinks water and grows.", ST_SOLID, TYPE_SOLID|PROP_NEUTPENETRATE|PROP_LIFE_DEC, &update_PLNT, NULL},
	{"ACID",	PIXPACK(0xED55FF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	40,		0,	0,	1,	1,	1,	10,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	34,		"Dissolves almost everything.", ST_LIQUID, TYPE_LIQUID|PROP_DEADLY, &update_ACID, &graphics_ACID},
	{"VOID",	PIXPACK(0x790B0B),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Hole, will drain away any particles.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"WTRV",	PIXPACK(0xA0A0FF),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	-0.1f,	0.75f,	0.0003f	* CFDS,	0,	0,		0,	0,	4,	1,	1,	1,		SC_GAS,			R_TEMP+100.0f+273.15f,	48,		"Steam, heats up air, produced from hot water.", ST_GAS, TYPE_GAS, &update_WTRV, NULL},
	{"CNCT",	PIXPACK(0xC0C0C0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	100,	"Concrete, stronger than stone.", ST_SOLID, TYPE_PART|PROP_HOT_GLOW, NULL, NULL},
	{"DSTW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	23,		"Distilled water, does not conduct electricity.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_DSTW, NULL},
	{"SALT",	PIXPACK(0xFFFFFF),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	75,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	110,	"Salt, dissolves in water.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"SLTW",	PIXPACK(0x4050F0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	35,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	75,		"Saltwater, conducts electricity, difficult to freeze.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_SLTW, NULL},
	{"DMND",	PIXPACK(0xCCFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	186,	"Diamond. Indestructible.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"BMTL",	PIXPACK(0x505070),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Breakable metal.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, &update_BMTL, NULL},
	{"BRMT",	PIXPACK(0x705060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	211,	"Broken metal.", ST_SOLID, TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"PHOT",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+900.0f+273.15f,	251,	"Photons. Travel in straight lines.", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_PHOT, &graphics_PHOT},
	{"URAN",	PIXPACK(0x707020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_NUCLEAR,		R_TEMP+30.0f+273.15f,	251,	"Heavy particles. Generates heat under pressure.", ST_SOLID, TYPE_PART | PROP_RADIOACTIVE, &update_URAN, NULL},
	{"WAX",		PIXPACK(0xF0F0BB),  0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	44,		"Wax. Melts at moderately high temperatures.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"MWAX",	PIXPACK(0xE0E0AA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	5,		0,	0,	2,	1,	1,	25,		SC_LIQUID,		R_TEMP+28.0f+273.15f,	44,		"Liquid Wax.", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"PSCN",	PIXPACK(0x805050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"P-Type Silicon, Will transfer current to any conductor.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NSCN",	PIXPACK(0x505080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"N-Type Silicon, Will not transfer current to P-Type Silicon.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"LN2",		PIXPACK(0x80A0DF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	0,	1,	1,	30,		SC_LIQUID,		70.15f,					70,		"Liquid Nitrogen. Very cold.", ST_SOLID, TYPE_LIQUID, NULL, NULL},
	{"INSL",	PIXPACK(0x9EA3B6),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	7,		0,	0,	10,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Insulator, does not conduct heat or electricity.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"VACU",	PIXPACK(0x303030),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.01f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+70.0f+273.15f,	255,	"Vacuum, sucks in other particles and heats up.", ST_NONE, TYPE_SOLID, NULL, NULL},
	{"VENT",	PIXPACK(0xEFEFEF),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.010f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP-16.0f+273.15f,	255,	"Air vent, creates pressure and pushes other particles away.", ST_NONE, TYPE_SOLID, NULL, NULL},
	{"RBDM",	PIXPACK(0xCCCCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	1,	50,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	240,	"Rubidium, explosive, especially on contact with water, low melting point", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"LRBD",	PIXPACK(0xAAAAAA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	1000,	1,	0,	2,	1,	1,	45,		SC_EXPLOSIVE,	R_TEMP+45.0f+273.15f,	170,	"Liquid Rubidium.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NTCT",	PIXPACK(0x505040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when hot (More than 100C)", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, &update_NPTCT, NULL},
	{"SAND",	PIXPACK(0xFFD090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Sand, Heavy particles. Meltable.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"GLAS",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	150,	"Solid. Meltable. Shatters under pressure", ST_SOLID, TYPE_SOLID | PROP_NEUTPASS | PROP_HOT_GLOW, &update_GLAS, NULL},
	{"PTCT",	PIXPACK(0x405050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when cold (Less than 100C)", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, &update_NPTCT, NULL},
	{"BGLA",	PIXPACK(0x606060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Glass, Heavy particles. Meltable. Bagels.", ST_SOLID, TYPE_PART | PROP_HOT_GLOW, NULL, NULL},
	{"THDR",	PIXPACK(0xFFFFA0),	0.0f,	0.00f * CFDS,	1.0f,	0.30f,	-0.99f,	0.6f,	0.62f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	1,		SC_EXPLOSIVE,	9000.0f		+273.15f,	1,	"Lightning! Very hot, inflicts damage upon most materials, transfers current to metals.", ST_NONE, TYPE_ENERGY, &update_THDR, &graphics_THDR},
	{"PLSM",	PIXPACK(0xBB99FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.30f,	0.001f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	1,		SC_GAS,			10000.0f		+273.15f,	5,	"Plasma, extremely hot.", ST_NONE, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_PYRO, &graphics_PLSM},
	{"ETRD",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electrode. Creates a surface that allows Plasma arcs. (Use sparingly)", ST_NONE, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"NICE",	PIXPACK(0xC0E0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0005f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		35.0f,					46,		"Nitrogen Ice.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"NBLE",	PIXPACK(0xEB4917),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	106,	"Noble Gas. Diffuses. Conductive. Ionizes into plasma when introduced to electricity", ST_GAS, TYPE_GAS|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"BTRY",	PIXPACK(0x858505),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Generates Electricity.", ST_SOLID, TYPE_SOLID, &update_BTRY, NULL},
	{"LCRY",	PIXPACK(0x505050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Liquid Crystal. Changes colour when charged. (PSCN Charges, NSCN Discharges)", ST_SOLID, TYPE_SOLID, &update_LCRY, &graphics_LCRY},
	{"STKM",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Stickman. Don't kill him!", ST_NONE, 0, &update_STKM, &graphics_STKM},
	{"SWCH",	PIXPACK(0x103B11),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f  * CFDS, 0,	0,		0,	0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Only conducts when switched on. (PSCN switches on, NSCN switches off)", ST_SOLID, TYPE_SOLID, &update_SWCH, &graphics_SWCH},
	{"SMKE",	PIXPACK(0x222222),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+320.0f+273.15f,	88,		"Smoke", ST_SOLID, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, NULL, &graphics_SMKE},
	{"DESL",	PIXPACK(0x440000),	1.0f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.0f,	0.0f	* CFDS,	2,	2,		0,	0,	5,	1,	1,	15,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Explodes under high pressure and temperatures", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"COAL",	PIXPACK(0x222222),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly.", ST_SOLID, TYPE_SOLID, &update_COAL, &graphics_COAL},
	{"LOXY",	PIXPACK(0x80A0EF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	5000,  	0,	0,	0,	1,	1,	30,		SC_LIQUID,		80.0f,					70,		"Liquid Oxygen. Very cold. Reacts with fire", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"OXYG",	PIXPACK(0x80A0FF),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.0f,	3.0f,	0.000f	* CFDS,	0,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,		 	R_TEMP+0.0f	+273.15f,   70,		"Gas. Ignites easily.", ST_GAS, TYPE_GAS, &update_O2, NULL},
	{"INWR",	PIXPACK(0x544141),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Insulated Wire. Doesn't conduct to metal or semiconductors.", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC, NULL, NULL},
	{"YEST",	PIXPACK(0xEEE0C0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	15,		0,	0,	30,	1,	1,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Yeast, grows when warm (~37C).", ST_SOLID, TYPE_PART, &update_YEST, NULL},
	{"DYST",	PIXPACK(0xBBB0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	20,		0,	0,	30,	0,	1,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Dead Yeast.", ST_SOLID, TYPE_PART, NULL, NULL},
	{"THRM",	PIXPACK(0xA08090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	211,	"Thermite. Burns at extremely high temperature.", ST_SOLID, TYPE_PART, &update_THRM, NULL},
	{"GLOW",	PIXPACK(0x445464),	0.3f,	0.02f * CFDS,	0.98f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	2,	1,	1,	40,		SC_LIQUID,		R_TEMP+20.0f+273.15f,	44,		"Glow, Glows under pressure", ST_LIQUID, TYPE_LIQUID|PROP_LIFE_DEC, &update_GLOW, &graphics_GLOW},
	{"BRCK",	PIXPACK(0x808080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Brick, breakable building material.", ST_SOLID, TYPE_SOLID|PROP_HOT_GLOW, NULL, NULL},
	{"CFLM",	PIXPACK(0x8080FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.0005f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	2,		SC_EXPLOSIVE,	0.0f,					88,		"Sub-zero flame.", ST_LIQUID, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, NULL, &graphics_HFLM},
	{"FIRW",	PIXPACK(0xFFA040),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	-0.99f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	55,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	70,		"Fireworks!", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_FIRW, &graphics_FIRW},
	{"FUSE",	PIXPACK(0x0A5706),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly. Ignites at somewhat high temperatures and electricity.", ST_SOLID, TYPE_SOLID, &update_FUSE, NULL},
	{"FSEP",	PIXPACK(0x63AD5F),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	70,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Fuse Powder. See FUSE.", ST_SOLID, TYPE_PART, &update_FSEP, NULL},
	{"AMTR",	PIXPACK(0x808080),	0.7f,   0.02f * CFDS,   0.96f,  0.80f,  0.00f,  0.10f,  1.00f,  0.0000f * CFDS, 0,	0,		0,	0,	0,	1,	1,	100,	SC_NUCLEAR,	 	R_TEMP+0.0f +273.15f,	70,		"Anti-Matter, Destroys a majority of particles", ST_NONE, TYPE_PART, &update_AMTR, NULL}, //Maybe TYPE_ENERGY?
	{"BCOL",	PIXPACK(0x333333),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Coal. Heavy particles. See COAL", ST_SOLID, TYPE_PART, &update_BCOL, NULL},
	{"PCLN",	PIXPACK(0x3B3B0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. When activated, duplicates any particles it touches.", ST_NONE, TYPE_SOLID, &update_PCLN, &graphics_PCLN},
	{"HSWC",	PIXPACK(0x3B0A0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Heat switch. Conducts Heat only when activated", ST_NONE, TYPE_SOLID, &update_HSWC, &graphics_HSWC},
	{"IRON",	PIXPACK(0x707070),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f	* CFDS, 0,	0,		0,	1,	50,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	251,	"Rusts with salt, can be used for electrolysis of WATR", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, &update_IRON, NULL},
	{"MORT",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_CRACKER2,	R_TEMP+4.0f	+273.15f,	60,		"Steam Train.", ST_NONE, TYPE_PART, &update_MORT, NULL},
	{"LIFE",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Game Of Life! B3/S23", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, &graphics_LIFE},
	{"DLAY",	PIXPACK(0x753590),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		4.0f+273.15f,			0,		"Conducts with temperature-dependent delay. (use HEAT/COOL).", ST_SOLID, TYPE_SOLID, &update_DLAY, &graphics_DLAY},
	{"CO2",		PIXPACK(0x666666),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.1f,	1.0f,	0.000f	* CFDS,	1,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+273.15f,			88,		"Carbon Dioxide", ST_GAS, TYPE_GAS, &update_CO2, NULL},
	{"DRIC",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0005f* CFDS,	0,	0,		0,	0,	20,	1,	1,	100,	SC_SOLIDS,		172.65f,				2,		"Dry Ice.", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"BUBW",	PIXPACK(0x2030D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Carbonated water. Conducts electricity. Freezes. Extinguishes fires.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE, &update_CBNW, &graphics_CBNW},
	{"STOR",	PIXPACK(0x50DFDF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Stores a single particle, releases when charged with PSCN, also passes to PIPE", ST_NONE, TYPE_SOLID, &update_STOR, &graphics_STOR},
	{"PVOD",	PIXPACK(0x792020),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Solid. When activated, destroys entering particles", ST_NONE, TYPE_SOLID, &update_PVOD, &graphics_PVOD},
	{"CONV",	PIXPACK(0x0AAB0A),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Converts whatever touches it into its ctype.", ST_NONE, TYPE_SOLID, &update_CONV, NULL},
	{"CAUS",	PIXPACK(0x80FFA0),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.0f,	1.50f,	0.000f	* CFDS,	0,	0,  	0,	0,	0,	1,	1,	1,		SC_GAS,		 	R_TEMP+0.0f	+273.15f,   70,		"Caustic Gas, acts like Acid", ST_GAS, TYPE_GAS|PROP_DEADLY, &update_CAUS, NULL},
	{"LIGH",	PIXPACK(0xFFFFC0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,	    "More realistic lighting. Set pen size for setting size of the lighting.", ST_SOLID, TYPE_SOLID, &update_LIGH, &graphics_LIGH},
    {"TESC",	PIXPACK(0x707040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Tesla coil!", ST_SOLID, TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
    {"DEST",	PIXPACK(0xFF3311),  -0.05f,	0.00f * CFDS,	0.95f,	0.95f,	-0.1f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,  0,	0,	1,	1,	101,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	150,	"More destructive Bomb.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_DEST, &graphics_DEST},
	{"SPNG",  	PIXPACK(0xFFBE30),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	20, 	0,  1, 	30,	1, 	1,	100,    SC_SOLIDS,  	R_TEMP+0.0f +273.15f,   251,    "A sponge, absorbs water.", ST_SOLID, TYPE_SOLID, &update_SPNG, &graphics_SPNG},
	{"RIME",  	PIXPACK(0xCCCCCC),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	0, 		0, 	0, 	30,	1,  1,	100,    SC_CRACKER2,  	243.15f,				100,    "Not quite Ice", ST_SOLID, TYPE_SOLID, &update_RIME, NULL},
	{"FOG",  	PIXPACK(0xAAAAAA),	0.8f,	0.00f * CFDS,	0.4f,	0.70f,	-0.1f,	0.0f,	0.99f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,	1,		SC_CRACKER2,  	243.15f,				100,    "Not quite Steam", ST_GAS, TYPE_GAS|PROP_LIFE_DEC, &update_FOG, NULL},
	{"BCLN",	PIXPACK(0xFFD040),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Breakable Clone.", ST_NONE, TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_BCLN, NULL},
	{"LOVE",	PIXPACK(0xFF30FF),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_CRACKER2,	373.0f,					40,		"Love...", ST_GAS, TYPE_SOLID, &update_MISC, NULL},
	{"DEUT",  	PIXPACK(0x00153F),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	31,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	251,	"Deuterium oxide. Volume changes with temp, radioactive with neutrons.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_DEUT, &graphics_DEUT},
	{"WARP",  	PIXPACK(0x000000),	0.8f,	0.00f * CFDS,	0.9f,	0.70f,	-0.1f,	0.0f,	3.00f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,	1,		SC_NUCLEAR,  	R_TEMP +273.15f,		100,    "Displaces other elements.", ST_GAS, TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL, &update_WARP, NULL},
	{"PUMP",	PIXPACK(0x0A0A3B),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	1,	100,	SC_POWERED,		273.15f,				0,		"Changes pressure to its temp when activated. (use HEAT/COOL).", ST_SOLID, TYPE_SOLID, &update_PUMP, &graphics_PUMP},
	{"FWRK",	PIXPACK(0x666666),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	1,	1,	1,	97,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	100,	"First fireworks made, activated by heat/neutrons.", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_FWRK, NULL},
	{"PIPE",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		273.15f,				0,		"Moves elements around, read FAQ on website for help.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_PIPE, &graphics_PIPE},
	{"FRZZ",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	1,	50,		SC_POWDERS,		90.0f,					46,		"FREEZE", ST_SOLID, TYPE_PART, &update_FRZZ, NULL},
	{"FRZW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_CRACKER2,	120.0f,					29,		"FREEZE WATER", ST_LIQUID, TYPE_LIQUID||PROP_LIFE_DEC, &update_FRZW, NULL},
	{"GRAV",	PIXPACK(0xFFE0A0),	0.7f,	0.00f * CFDS,	1.00f,	1.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Changes colour based on velocity.", ST_SOLID, TYPE_PART, &update_MISC, &graphics_GRAV},
	{"BIZR",	PIXPACK(0x00FF77),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	30,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	29,		"Bizarre... contradicts the normal state changes.", ST_LIQUID, TYPE_LIQUID, &update_BIZR, &graphics_BIZR},
	{"BIZG",	PIXPACK(0x00FFBB),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	2.75f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_CRACKER2,	R_TEMP-200.0f+273.15f,	42,		"Bizarre gas", ST_GAS, TYPE_GAS, &update_BIZR, &graphics_BIZR},
	{"BIZS",	PIXPACK(0x00E455),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_CRACKER2,	R_TEMP+300.0f+273.15f,	251,	"Bizarre solid", ST_SOLID, TYPE_SOLID, &update_BIZR, &graphics_BIZR},
	{"INST",	PIXPACK(0x404039),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Instantly conducts, PSCN to charge, NSCN to take.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, NULL, NULL},
	{"ISOZ",	PIXPACK(0xAA30D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	0,	1,	1,	24,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	29,		"Radioactive liquid", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE, &update_ISZ, NULL},
	{"ISZS",	PIXPACK(0x662089),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0007f* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_NUCLEAR,		140.00f,				251,	"Solid form of ISOZ, slowly decays.", ST_SOLID, TYPE_SOLID, &update_ISZ, NULL},
	{"PRTI",	PIXPACK(0xEB5917),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Portal IN.  Things go in here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID, &update_PRTI, &graphics_PRTI},
	{"PRTO",	PIXPACK(0x0020EB),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Portal OUT.  Things come out here, now with channels (same as WIFI)", ST_SOLID, TYPE_SOLID, &update_PRTO, &graphics_PRTO},
	{"PSTE",	PIXPACK(0xAA99AA),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	31,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Colloid, Hardens under pressure", ST_LIQUID, TYPE_LIQUID, NULL, NULL},
	{"PSTS",	PIXPACK(0x776677),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	20,	0,	1,	100,	SC_CRACKER,		R_TEMP-2.0f	+273.15f,	29,		"Solid form of PSTE, temporary", ST_SOLID, TYPE_SOLID, NULL, NULL},
	{"ANAR",	PIXPACK(0xFFFFEE),	-0.7f,	-0.02f * CFDS,	0.96f,	0.80f,	0.1f,	-0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Behaves opposite gravity", ST_SOLID, TYPE_PART, &update_ANAR, NULL},
	{"VINE",	PIXPACK(0x079A00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	65,		"Vine, grows", ST_SOLID, TYPE_SOLID, &update_VINE, NULL},
	{"INVS",	PIXPACK(0x00CCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	15,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Invisible to everything while under pressure.", ST_SOLID, TYPE_SOLID | PROP_NEUTPASS, NULL, &graphics_INVS},
	{"EQVE",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	0,	1,	85,		SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	70,		"Shared velocity test", ST_SOLID, TYPE_PART, NULL, NULL},
	{"SPWN2",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"STK2 spawn point", ST_SOLID, TYPE_SOLID, &update_SPAWN2, NULL},
	{"SPWN",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"STKM spawn point", ST_SOLID, TYPE_SOLID, &update_SPAWN, NULL},
	{"SHLD",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"Shield, spark it to grow", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD1, NULL},
	{"SHD2",	PIXPACK(0x777777),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 2", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD2, NULL},
	{"SHD3",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 3", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD3, NULL},
	{"SHD4",	PIXPACK(0x212121),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	1,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 4", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_SHLD4, NULL},
	{"LOLZ",	PIXPACK(0x569212),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_CRACKER2,	373.0f,					40,		"Lolz", ST_GAS, TYPE_SOLID, &update_MISC, NULL},
	{"WIFI",	PIXPACK(0x40A060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	2,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,		"Wireless transmitter, color coded.", ST_SOLID, TYPE_SOLID, &update_WIFI, &graphics_WIFI},
	{"FILT",	PIXPACK(0x000056),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Filter for photons, changes the color.", ST_SOLID, TYPE_SOLID, NULL, &graphics_FILT},
	{"ARAY",	PIXPACK(0xFFBB00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	251,	"Ray Emitter. Rays create points when they collide", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_ARAY, NULL},
	{"BRAY",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	0,	1,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	251,	"Ray Point. Rays create points when they collide", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL, NULL, &graphics_BRAY},
	{"STK2",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Stickman. Don't kill him!", ST_NONE, 0, &update_STKM2, &graphics_STKM2},
	{"BOMB",	PIXPACK(0xFFF288),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	20,	1,	1,	30,		SC_EXPLOSIVE,	R_TEMP-2.0f	+273.15f,	29,		"Bomb.", ST_NONE, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_BOMB, &graphics_BOMB},
	{"C-5",		PIXPACK(0x2050E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Cold explosive", ST_SOLID, TYPE_SOLID | PROP_NEUTPENETRATE, &update_C5, NULL},
	{"SING",	PIXPACK(0x242424),	0.7f,	0.36f * CFDS,	0.96f,	0.80f,	0.1f,	0.12f,	0.00f,	-0.001f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	86,		SC_NUCLEAR,		R_TEMP+0.0f	+273.15f,	70,		"Singularity", ST_SOLID, TYPE_PART|PROP_LIFE_DEC, &update_SING, NULL},
	{"QRTZ",	PIXPACK(0xAADDDD),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	3,		"Quartz, breakable mineral. Conducts but becomes brittle at lower temperatures.", ST_SOLID, TYPE_SOLID|PROP_HOT_GLOW|PROP_LIFE_DEC, &update_QRTZ, &graphics_QRTZ},
	{"PQRT",	PIXPACK(0x88BBBB),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.27f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	3,		"Broken quartz.", ST_SOLID, TYPE_PART| PROP_HOT_GLOW, &update_QRTZ, &graphics_QRTZ},
	{"EMP",	    PIXPACK(0x66AAFF),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	3,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	121,	"Breaks activated electronics.", ST_SOLID, TYPE_SOLID|PROP_LIFE_DEC, &update_EMP, &graphics_EMP},
	{"BREL",	PIXPACK(0x707060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.18f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	211,	"Broken electronics", ST_SOLID, TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW, NULL, NULL},
	{"ELEC",	PIXPACK(0xDFEFFF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	-1,		SC_NUCLEAR,		R_TEMP+200.0f+273.15f,	251,	"Electrons", ST_GAS, TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_ELEC, &graphics_ELEC},
	{"ACEL",	PIXPACK(0x0099CC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Accelerator", ST_NONE, TYPE_SOLID, &update_ACEL, &graphics_ACEL},
	{"DCEL",	PIXPACK(0x99CC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Decelerator", ST_NONE, TYPE_SOLID, &update_DCEL, &graphics_DCEL},
	/*FREE*/{"REPL",	PIXPACK(0x259588),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"B1357/S1357", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"MYST",	PIXPACK(0x0C3C00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"B3458/S05678", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	{"BOYL",	PIXPACK(0x0A3200),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.18f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	42,		"Boyle, variable pressure gas. Expands when heated.", ST_GAS, TYPE_GAS, &update_BOYL, NULL},
	/*FREE*/{"LOTE",	PIXPACK(0xFF0000),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"Behaves kinda like Living on the Edge S3458/B37/4", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"FRG2",	PIXPACK(0x00FF00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"Like Frogs rule S124/B3/3", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"STAR",	PIXPACK(0x0000FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"Like Star Wars rule S3456/B278/6", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"FROG",	PIXPACK(0x00AA00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"Frogs S12/B34/3", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"BRAN",	PIXPACK(0xCCCC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	0,	0,	100,	SC_LIFE,		9000.0f,				40,		"Brian 6 S6/B246/3", ST_NONE, TYPE_SOLID|PROP_LIFE, NULL, NULL},
	/*FREE*/{"WIND",	PIXPACK(0x101010),  0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,	0.000f  * CFDS,	0,  0,		0,  0,  0,  0,  0,	100,	SC_SPECIAL,		0.0f,					40,		"", ST_NONE, ST_NONE, NULL, NULL},
	{"HYGN",		PIXPACK(0x5070FF),	2.0f,	0.00f * CFDS,	0.99f,	0.30f,	-0.10f,	0.00f,	3.00f,	0.000f	* CFDS, 0,  0,		0,	0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+0.0f +273.15f,	251,	"Combines with O2 to make WATR", ST_GAS, TYPE_GAS, &update_H2, NULL},
	{"SOAP",	PIXPACK(0xF5F5DC),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	35,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Soap. Creates bubbles.", ST_LIQUID, TYPE_LIQUID|PROP_NEUTPENETRATE|PROP_LIFE_DEC, &update_SOAP, NULL},
	{"BHOL",	PIXPACK(0x202020),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"Black hole (Requires newtonian gravity)", ST_SOLID, TYPE_SOLID, &update_NBHL, NULL},
	{"WHOL",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"White hole (Requires newtonian gravity)", ST_SOLID, TYPE_SOLID, &update_NWHL, NULL},
	{"MERC",    PIXPACK(0x736B6D),	0.4f,	0.04f * CFDS,	0.94f,	0.80f,	0.0f,	0.3f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	1,	91,		SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Mercury. Volume changes with temperature, Conductive.", ST_LIQUID, TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTABSORB|PROP_LIFE_DEC, &update_MERC, NULL},
	{"PBCN",	PIXPACK(0x3B1D0A),	0.0f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	1,	100,	SC_POWERED,		R_TEMP+0.0f	+273.15f,	251,	"Powered breakable clone", ST_NONE, TYPE_SOLID, &update_PBCN, &graphics_PBCN},
	{"GPMP",	PIXPACK(0x0A3B3B),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	1,	100,	SC_POWERED,		0.0f		+273.15f,	0,		"Changes gravity to its temp when activated. (use HEAT/COOL).", ST_NONE, TYPE_SOLID, &update_GPMP, &graphics_GPMP},
	{"CLST",	PIXPACK(0xE4A4A4),	0.7f,	0.02f * CFDS,	0.94f,	0.95f,	0.0f,	0.2f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Clay dust. Produces paste when mixed with water.", ST_SOLID, TYPE_PART, &update_CLST, &graphics_CLST},	
	{"WIRE",    PIXPACK(0xFFCC00),  0.0f,   0.00f * CFDS,   0.00f,  0.00f,  0.0f,   0.0f,   0.00f,  0.000f  * CFDS, 0,  0,      0,  0,  0,  1,  1,  100,    SC_ELEC,        R_TEMP+0.0f +273.15f,   250,    "WireWorld wires.",ST_SOLID,TYPE_SOLID,&update_WIRE, &graphics_WIRE},
	{"GBMB",	PIXPACK(0x1144BB),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	20,	1,	1,	30,		SC_EXPLOSIVE,	R_TEMP-2.0f	+273.15f,	29,		"Sticks to first object it touches then produces strong gravity push.", ST_NONE, TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC, &update_GBMB, &graphics_GBMB},
	{"FIGH",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Fighter. Tries to kill stickmans.", ST_NONE, 0, &update_FIGH, &graphics_FIGH},
	//Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd M	Use	Weight	Section			H						Ins		Description
};

// temporarily define abbreviations for impossible p/t values
#define IPL -257.0f
#define IPH 257.0f
#define ITL MIN_TEMP-1
#define ITH MAX_TEMP+1
// no transition (PT_NONE means kill part)
#define NT -1
// special transition - lava ctypes etc need extra code, which is only found and run if ST is given
#define ST PT_NUM
static part_transition ptransitions[PT_NUM] =
{	//			if low pressure		if high pressure	if low temperature	if high temperature
	// Name		plv		plt	 		phv		pht			tlv		tlt	 		thv		tht
	/* NONE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DUST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WATR */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* OIL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			333.0f,	PT_GAS},
	/* FIRE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2773.0f,PT_PLSM},
	/* STNE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			983.0f,	PT_LAVA},
	/* LAVA */ {IPL,	NT,			IPH,	NT,			2573.15f,ST,		ITH,	NT}, // 2573.15f is highest melt pt of possible ctypes
	/* GUN  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* NITR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* CLNE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GAS  */ {IPL,	NT,			6.0f,	PT_OIL,		ITL,	NT,			573.0f,	PT_FIRE},
	/* C-4  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			673.0f,	PT_FIRE},
	/* GOO  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ICE  */ {IPL,	NT,			0.8f,	PT_SNOW,	ITL,	NT,			233.0f,	ST},
	/* METL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_LAVA},
	/* SPRK */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SNOW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			273.0f,	PT_WATR},
	/* WOOD */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			873.0f,	PT_FIRE},
	/* NEUT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLUT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLNT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			573.0f,	PT_FIRE},
	/* ACID */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VOID */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WTRV */ {IPL,	NT,			IPH,	NT,			371.0f,	ST,			ITH,	NT},
	/* CNCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1123.0f,PT_LAVA},
	/* DSTW */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* SALT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1173.0f,PT_LAVA},
	/* SLTW */ {IPL,	NT,			IPH,	NT,			233.0f,	PT_ICEI,	483.0f,	ST},
	/* DMND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BMTL */ {IPL,	NT,			1.0f,	ST,			ITL,	NT,			1273.0f,PT_LAVA},
	/* BRMT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_LAVA},
	/* PHOT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* URAN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WAX  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			319.0f,	PT_MWAX},
	/* MWAX */ {IPL,	NT,			IPH,	NT,			318.0f,	PT_WAX,		673.0f,	PT_FIRE},
	/* PSCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* NSCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* LN2  */ {IPL,	NT,			IPH,	NT,			63.0f,	PT_NICE,	77.0f,	PT_NONE},
	/* INSL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VACU */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VENT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* RBDM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			312.0f,	PT_LRBD},
	/* LRBD */ {IPL,	NT,			IPH,	NT,			311.0f,	PT_RBDM,	961.0f,	PT_FIRE},
	/* NTCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* SAND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* GLAS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* PTCT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1414.0f,PT_LAVA},
	/* BGLA */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1973.0f,PT_LAVA},
	/* THDR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PLSM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ETRD */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* NICE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			63.1f,	PT_LNTG},
	/* NBLE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BTRY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2273.0f,PT_PLSM},
	/* LCRY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1273.0f,PT_BGLA},
	/* STKM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
	/* SWCH */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SMKE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			625.0f,	PT_FIRE},
	/* DESL */ {IPL,	NT,			5.0f,	PT_FIRE,	ITL,	NT,			335.0f,	PT_FIRE},
	/* COAL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LO2  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			90.1f,	PT_O2},
	/* O2   */ {IPL,	NT,			IPH,	NT,			90.0f,	PT_LO2,		ITH,	NT},
	/* INWR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* YEST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			373.0f,	PT_DYST},
	/* DYST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			473.0f,	PT_DUST},
	/* THRM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GLOW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BRCK */ {IPL,	NT,			8.8f,	PT_STNE,	ITL,	NT,			1223.0f,PT_LAVA},
	/* CFLM */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FIRW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FUSE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FSEP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* AMTR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BCOL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PCLN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* HSWC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* IRON */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1687.0f,PT_LAVA},
	/* MORT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LIFE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DLAY */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* CO2  */ {IPL,    NT,         IPH,    NT,         194.65f,PT_DRIC,   ITH,	NT},
	/* DRIC */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			195.65f,PT_CO2},
	/* CBNW */ {IPL,	NT,			IPH,	NT,			273.15f,PT_ICEI,	373.0f,	PT_WTRV},
	/* STOR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* STOR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPNG */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2730.0f,PT_FIRE},
	/* RIME */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			273.15f,PT_WATR},
	/* FOG  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			373.15f,PT_WTRV},
	/* BCLN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* LOVE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* DEUT  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WARP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PUMP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FWRK */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PIPE */ {IPL,	NT,			10.0f,	PT_BRMT,	ITL,	NT,			ITH,	NT},
	/* FRZZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FRZW */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			53.0f,	PT_ICEI},
	/* GRAV */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BIZR */ {IPL,	NT,			IPH,	NT,			100.0f,	PT_BIZRG,	400.0f,	PT_BIZRS},
	/* BIZRG*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			100.0f,	PT_BIZR},//,	400.0f,	PT_BIZRS},
	/* BIZRS*/ {IPL,	NT,			IPH,	NT,			400.0f,	PT_BIZR,	ITH,	NT},//	100.0f,	PT_BIZRG},
	/* INST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ISOZ */ {IPL,	NT,			IPH,	NT,			160.0f,	PT_ISZS,	ITH,	NT},
	/* ISZS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			300.0f,	PT_ISOZ},
	/* PRTI */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PRTO */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* PSTE */ {IPL,	NT,			0.5f,	PT_PSTS,	ITL,	NT,			747.0f,	PT_BRCK},
	/* PSTS */ {0.5f,	PT_PSTE,	IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ANAR */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* VINE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			573.0f,	PT_FIRE},
	/* INVS */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* EQVE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPWN2*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SPAWN*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SHLD1*/ {IPL,	NT,			7.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD2*/ {IPL,	NT,			15.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD3*/ {IPL,	NT,			25.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* SHLD4*/ {IPL,	NT,			40.0f,	PT_NONE,	ITL,	NT,			ITH,	NT},
	/* LOlZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WIFI */ {IPL,	NT,			15.0f,	PT_BRMT,	ITL,	NT,			ITH,	NT},
	/* FILT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* ARAY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BRAY */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* STKM2*/ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
	/* BOMB */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* C-5  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SING */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* QRTZ */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2573.15f,PT_LAVA},
	/* PQRT */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			2573.15f,PT_LAVA},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* BOYL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/*FREE*//* GOL  */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* WIND */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* H2   */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* SOAP */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITL,	NT},
	/* NBHL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* NWHL */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* MERC */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* PBCN */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GPMP */ {IPL,    NT,         IPH,    NT,         ITL,    NT,         ITH,	NT},
	/* CLST */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			1256.0f,	PT_LAVA},
	/* WIRE */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* GBMB */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			ITH,	NT},
	/* FIGH */ {IPL,	NT,			IPH,	NT,			ITL,	NT,			620.0f,	PT_FIRE},
};
#undef IPL
#undef IPH
#undef ITL
#undef ITH
#undef NT
#undef ST

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

extern int gravwl_timeout;

extern playerst player;
extern playerst player2;

extern playerst fighters[256];
extern unsigned char fighcount;

extern int gravityMode;
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

void gravity_mask();

#endif
