#ifndef POWDER_H
#define POWDER_H

#include "graphics.h"
#include "defines.h"
#include "interface.h"

#define CM_COUNT 10
#define CM_CRACK 9
#define CM_GRAD 8
#define CM_NOTHING 7
#define CM_FANCY 6
#define CM_HEAT 5
#define CM_BLOB 4
#define CM_FIRE 3
#define CM_PERS 2
#define CM_PRESS 1
#define CM_VEL 0


#define UI_WALLSTART 222
#define UI_ACTUALSTART 122
#define UI_WALLCOUNT 19

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

#define WL_ALLOWGAS	140



#define PT_NONE	0
#define PT_DUST	1
#define PT_WATR	2
#define PT_OIL 3
#define PT_FIRE 4
#define PT_STNE 5
#define PT_LAVA 6
#define PT_GUNP	7
#define PT_NITR	8
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
#define PT_GOL 78
#define PT_HLIF 79
#define PT_ASIM 80
#define PT_2x2 81
#define PT_DANI 82
#define PT_AMOE 83
#define PT_MOVE 84
#define PT_PGOL 85
#define PT_DMOE 86
#define PT_34 87
#define PT_LLIF 88
#define PT_STAN 89
#define PT_CRAC 90
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
#define PT_EQUALVEL 116//all particles equal their velocities
#define PT_INST2 117//blank element, replace with something new, has to be here for numbering
#define PT_INST3 118//blank same as above
#define PT_SHLD1 119
#define PT_SHLD2 120
#define PT_SHLD3 121
#define PT_SHLD4 122
#define PT_LOLZ 123
#define PT_WIFI 124
#define PT_FILT 125
#define PT_ARAY 126
#define PT_BRAY 127
#define PT_HETR 128 
#define PT_NUM  129

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
#define PROP_CONDUCTS		0x0020 //32 Conducts electricity (Currently Unused)
#define PROP_BLACK			0x0040 //64 Absorbs Photons
#define PROP_NEUTPENETRATE	0x0080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x0100 //256 Absorbs neutrons, reflect is default
#define PROP_NEUTPASS		0x0200 //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x0400 //1024 Is deadly for stickman.
#define FLAG_STAGNANT	1

struct particle
{
    int type;
    int life, ctype;
    float x, y, vx, vy;
    float temp;
    float pavg[2];
    int flags;
    int tmp;
};
typedef struct particle particle;

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
    int weight;
    int menusection;
    float heat;
    unsigned char hconduct;
    const char *descs;
    const unsigned short properties;
};
typedef struct part_type part_type;

struct part_state
{
    char state;
    int solid;
    float stemp;
    int liquid;
    float ltemp;
    int gas;
    float gtemp;
    int burn;
    float btemp;
};

typedef struct part_state part_state;
//Should probably be const.
/* Weight Help
 * 1   = Gas   ||
 * 2   = Light || Liquids  0-49
 * 98  = Heavy || Powder  50-99
 * 100 = Solid ||
 * -1 is Neutrons and Photons
 */
static const part_type ptypes[PT_NUM] =
{
    //Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd M	Weight	Section			H						Ins		Description
    {"",		PIXPACK(0x000000),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Erases particles.", 0},
    {"DUST",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Flammable.", TYPE_PART},
    {"WATR",	PIXPACK(0x2030D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Liquid. Conducts electricity. Freezes. Extinguishes fires.", TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTPENETRATE},
    {"OIL",		PIXPACK(0x404010),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	20,		0,	0,	5,	1,	20,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Flammable.", TYPE_LIQUID},
    {"FIRE",	PIXPACK(0xFF1000),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	2,		SC_EXPLOSIVE,	R_TEMP+400.0f+273.15f,	88,		"Ignites flammable materials. Heats air.", TYPE_GAS},
    {"STNE",	PIXPACK(0xA0A0A0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Heavy particles. Meltable.", TYPE_PART},
    {"LAVA",	PIXPACK(0xE05010),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.0003f	* CFDS,	2,	0,		0,	0,	2,	1,	45,		SC_LIQUID,		R_TEMP+1500.0f+273.15f,	60,		"Heavy liquid. Ignites flammable materials. Solidifies when cold.", TYPE_LIQUID},
    {"GUN",		PIXPACK(0xC0C0D0),	0.7f,	0.02f * CFDS,	0.94f,	0.80f,	-0.1f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	600,	1,	0,	10,	1,	85,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	97,		"Light dust. Explosive.", TYPE_PART},
    {"NITR",	PIXPACK(0x20E010),	0.5f,	0.02f * CFDS,	0.92f,	0.97f,	0.0f,	0.2f,	0.00f,	0.000f	* CFDS,	2,	1000,	2,	0,	3,	1,	23,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	50,		"Liquid. Pressure sensitive explosive.", TYPE_LIQUID},
    {"CLNE",	PIXPACK(0xFFD010),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Duplicates any particles it touches.", TYPE_SOLID},
    {"GAS",		PIXPACK(0xE0FF20),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	600,	0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	42,		"Gas. Diffuses. Flammable. Liquifies under pressure.", TYPE_GAS},
    {"C-4",		PIXPACK(0xD080E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	2,	50,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Solid. Pressure sensitive explosive.", TYPE_SOLID | PROP_NEUTPENETRATE},
    {"GOO",		PIXPACK(0x804000),	0.1f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	75,		"Solid. Deforms and disappears under pressure.", TYPE_SOLID | PROP_NEUTPENETRATE},
    {"ICE",		PIXPACK(0xA0C0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	20,	1,	100,	SC_SOLIDS,		R_TEMP-50.0f+273.15f,	46,		"Solid. Freezes water. Crushes under pressure. Cools down air.", TYPE_SOLID},
    {"METL",	PIXPACK(0x404060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Conducts electricity. Meltable.", TYPE_SOLID|PROP_CONDUCTS},
    {"SPRK",	PIXPACK(0xFFFF80),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electricity. Conducted by metal and water.", TYPE_SOLID},
    {"SNOW",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	50,		SC_POWDERS,		R_TEMP-30.0f+273.15f,	46,		"Light particles.", TYPE_PART},
    {"WOOD",	PIXPACK(0xC0A040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	15,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Solid. Flammable.", TYPE_SOLID},
    {"NEUT",	PIXPACK(0x20E0FF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	-1,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	60,		"Neutrons. Interact with matter in odd ways.", TYPE_ENERGY},
    {"PLUT",	PIXPACK(0x407020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	90,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	251,	"Heavy particles. Fissile. Generates neutrons under pressure.", TYPE_PART|PROP_NEUTPENETRATE},
    {"PLNT",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	65,		"Plant, drinks water and grows.", TYPE_SOLID},
    {"ACID",	PIXPACK(0xED55FF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	40,		0,	0,	1,	1,	10,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	34,		"Dissolves almost everything.", TYPE_LIQUID},
    {"VOID",	PIXPACK(0x790B0B),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0003f* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Hole, will drain away any particles.", TYPE_SOLID},
    {"WTRV",	PIXPACK(0xA0A0FF),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	-0.1f,	0.75f,	0.0003f	* CFDS,	0,	0,		0,	0,	4,	1,	1,		SC_GAS,			R_TEMP+100.0f+273.15f,	48,		"Steam, heats up air, produced from hot water.", TYPE_GAS},
    {"CNCT",	PIXPACK(0xC0C0C0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	100,	"Concrete, stronger than stone.", TYPE_PART},
    {"DSTW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	30,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	23,		"Distilled water, does not conduct electricity.", TYPE_LIQUID|PROP_NEUTPENETRATE},
    {"SALT",	PIXPACK(0xFFFFFF),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	75,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	110,	"Salt, dissolves in water.", TYPE_PART},
    {"SLTW",	PIXPACK(0x4050F0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	35,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	75,		"Saltwater, conducts electricity, difficult to freeze.", TYPE_LIQUID|PROP_CONDUCTS|PROP_NEUTPENETRATE},
    {"DMND",	PIXPACK(0xCCFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	186,	"Diamond. Indestructable.", TYPE_SOLID}, //ief015 - Added diamond. Because concrete blocks are kinda pointless.
    {"BMTL",	PIXPACK(0x505070),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Breakable metal.", TYPE_SOLID|PROP_CONDUCTS},
    {"BRMT",	PIXPACK(0x705060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	211,	"Broken metal.", TYPE_PART|PROP_CONDUCTS},
    {"PHOT",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	-1,		SC_ELEC,		R_TEMP+900.0f+273.15f,	251,	"Photons. Travel in straight lines.", TYPE_ENERGY},
    {"URAN",	PIXPACK(0x707020),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	0,	1,	90,		SC_NUCLEAR,		R_TEMP+30.0f+273.15f,	251,	"Heavy particles. Generates heat under pressure.", TYPE_PART},
    {"WAX",		PIXPACK(0xF0F0BB),  0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	44,		"Wax. Melts at moderately high temperatures.", TYPE_SOLID},
    {"MWAX",	PIXPACK(0xE0E0AA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	5,		0,	0,	2,	1,	25,		SC_LIQUID,		R_TEMP+28.0f+273.15f,	44,		"Liquid Wax.", TYPE_LIQUID},
    {"PSCN",	PIXPACK(0x805050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"P-Type Silicon, Will transfer current to any conductor.", TYPE_SOLID},
    {"NSCN",	PIXPACK(0x505080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"N-Type Silicon, Will only transfer current to P-Type Silicon.", TYPE_SOLID},
    {"LN2",		PIXPACK(0x80A0DF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	0,	1,	30,		SC_LIQUID,		70.15f,					70,		"Liquid Nitrogen. Very cold.", TYPE_LIQUID},
    {"INSL",	PIXPACK(0x9EA3B6),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	7,		0,	0,	10,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	0,		"Insulator, does not conduct heat or electricity.", TYPE_SOLID},
    {"BHOL",	PIXPACK(0x202020),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.01f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SPECIAL,		R_TEMP+70.0f+273.15f,	255,	"Black hole, sucks in other particles and heats up.", TYPE_SOLID},
    {"WHOL",	PIXPACK(0xEFEFEF),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.010f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SPECIAL,		R_TEMP-16.0f+273.15f,	255,	"White hole, pushes other particles away.", TYPE_SOLID},
    {"RBDM",	PIXPACK(0xCCCCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	1,	50,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	240,	"Rubidium, explosive, especially on contact with water, low melting point", TYPE_SOLID|PROP_CONDUCTS},
    {"LRBD",	PIXPACK(0xAAAAAA),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000001f* CFDS,2,	1000,	1,	0,	2,	1,	45,		SC_EXPLOSIVE,	R_TEMP+45.0f+273.15f,	170,	"Liquid Rubidium.", TYPE_LIQUID|PROP_CONDUCTS},
    {"NTCT",	PIXPACK(0x505040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when hot (More than 100C)", TYPE_SOLID},
    {"SAND",	PIXPACK(0xFFD090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	1,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Sand, Heavy particles. Meltable.", TYPE_PART},
    {"GLAS",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	150,	"Solid. Meltable. Shatters under pressure", TYPE_SOLID | PROP_NEUTPASS},
    {"PTCT",	PIXPACK(0x405050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Semi-conductor. Only conducts electricity when cold (Less than 120C)", TYPE_SOLID},
    {"BGLA",	PIXPACK(0x606060),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Glass, Heavy particles. Meltable. Bagels.", TYPE_PART},
    {"THDR",	PIXPACK(0xFFFFA0),	0.0f,	0.00f * CFDS,	1.0f,	0.30f,	-0.99f,	0.6f,	0.62f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	1,		SC_ELEC,		9000.0f		+273.15f,	251,	"Lightning! Very hot, inflicts damage upon most materials, transfers current to metals.", TYPE_ENERGY},
    {"PLSM",	PIXPACK(0xBB99FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.30f,	0.001f	* CFDS,	0,	0,		0,	0,	0,	1,	1,		SC_GAS,			9000.0f		+273.15f,	115,	"Plasma, extremely hot.", TYPE_GAS},
    {"ETRD",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Electrode. Creates a surface that allows Plasma arcs. (Use sparingly)", TYPE_SOLID},
    {"NICE",	PIXPACK(0xC0E0FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0005f* CFDS,	0,	0,		0,	0,	20,	1,	100,	SC_SOLIDS,		35.0f,					46,		"Nitrogen Ice.", TYPE_SOLID},
    {"NBLE",	PIXPACK(0xEB4917),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	0.75f,	0.001f	* CFDS,	0,	0,		0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+2.0f	+273.15f,	106,	"Noble Gas. Diffuses. Conductive. Ionizes into plasma when intruduced to electricity", TYPE_GAS},
    {"BTRY",	PIXPACK(0x858505),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Generates Electricity.", TYPE_SOLID},
    {"LCRY",	PIXPACK(0x505050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Liquid Crystal. Changes colour when charged. (PSCN Charges, NSCN Discharges)", TYPE_SOLID},
    {"STKM",	PIXPACK(0x000000),	0.5f,	0.00f * CFDS,	0.2f,	1.0f,	0.0f,	0.0f,	0.0f,	0.00f	* CFDS,	0,	0,		0,	0,	0,	1,	50,		SC_SPECIAL,		R_TEMP+14.6f+273.15f,	0,		"Stickman. Don't kill him!", 0},
    {"SWCH",	PIXPACK(0x103B11),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f  * CFDS, 0,	0,		0,	0,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. Only conducts when switched on. (PSCN switches on, NSCN switches off)", TYPE_SOLID},
    {"SMKE",	PIXPACK(0x222222),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.001f	* CFDS,	1,	0,		0,	0,	1,	1,	1,		SC_GAS,			R_TEMP+320.0f+273.15f,	88,		"Smoke", TYPE_GAS},
    {"DESL",	PIXPACK(0x440000),	1.0f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.0f,	0.0f	* CFDS,	2,	2,		0,	0,	5,	1,	15,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	42,		"Liquid. Explodes under high pressure and temperatures", TYPE_LIQUID},
    {"COAL",	PIXPACK(0x222222),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly.", TYPE_SOLID},
    {"LO2",		PIXPACK(0x80A0EF),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	5000,  	0,	0,	0,	1,	30,		SC_LIQUID,		80.0f,					70,		"Liquid Oxygen. Very cold. Reacts with fire", TYPE_LIQUID},
    {"O2",		PIXPACK(0x80A0FF),	2.0f,   0.00f * CFDS,   0.99f,	0.30f,	-0.1f,	0.0f,	3.0f,	0.000f	* CFDS,	0,	1000,  	0,	0,	0,	1,	1,		SC_GAS,		 	R_TEMP+0.0f	+273.15f,   70,		"Gas. Ignites easily.", TYPE_GAS},
    {"INWR",	PIXPACK(0x544141),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Insulated Wire. Doesn't conduct to metal or semiconductors.", TYPE_SOLID},
    {"YEST",	PIXPACK(0xEEE0C0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	15,		0,	0,	30,	1,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Yeast, grows when warm (~37C).", TYPE_PART},
    {"DYST",	PIXPACK(0xBBB0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	20,		0,	0,	30,	0,	80,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Dead Yeast.", TYPE_PART},
    {"THRM",	PIXPACK(0xA08090),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	2,	1,	90,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	211,	"Thermite. Burns at extremly high temperature.", TYPE_PART},
    {"GLOW",	PIXPACK(0x445544),	0.3f,	0.02f * CFDS,	0.95f,	0.80f,	0.0f,	0.15f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	2,	1,	40,		SC_LIQUID,		R_TEMP+20.0f+273.15f,	44,		"Glow, Glows under pressure", TYPE_LIQUID},
    {"BRCK",	PIXPACK(0x808080),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Brick, breakable building material.", TYPE_SOLID},
    {"CFLM",	PIXPACK(0x8080FF),	0.9f,	0.04f * CFDS,	0.97f,	0.20f,	0.0f,	-0.1f,	0.00f,	0.0005f	* CFDS,	1,	0,		0,	0,	1,	1,	2,		SC_SPECIAL,		0.0f,					88,		"Sub-zero flame.", TYPE_GAS},
    {"FIRW",	PIXPACK(0xFFA040),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	-0.99f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	55,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	70,		"Fireworks!", TYPE_PART},
    {"FUSE",	PIXPACK(0x0A5706),	0.0f,   0.00f * CFDS,   0.90f,  0.00f,  0.0f,   0.0f,   0.0f,   0.0f	* CFDS, 0,	0,		0,	0,	20,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	200,	"Solid. Burns slowly. Ignites at somewhat high temperatures and electricity.", TYPE_SOLID},
    {"FSEP",	PIXPACK(0x63AD5F),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	1,	70,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Fuse Powder. See FUSE.", TYPE_PART},
    {"AMTR",	PIXPACK(0x808080),	0.7f,   0.02f * CFDS,   0.96f,  0.80f,  0.00f,  0.10f,  1.00f,  0.0000f * CFDS, 0,	0,		0,	0,	0,	1,	100,	SC_NUCLEAR,	 	R_TEMP+0.0f +273.15f,	70,		"Anti-Matter, Destroys a majority of particles", TYPE_PART}, //Maybe TYPE_ENERGY?
    {"BCOL",	PIXPACK(0x333333),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	5,	2,	1,	90,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	150,	"Broken Coal. Heavy particles. See COAL", TYPE_PART},
    {"PCLN",	PIXPACK(0x3B3B10),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Solid. When actived, duplicates any particles it touches.", TYPE_SOLID},
    {"HSWC",	PIXPACK(0x3B1010),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	251,	"Heat switch. Conducts Heat only when activated", TYPE_SOLID},
    {"IRON",	PIXPACK(0x707070),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f	* CFDS, 0,	0,		0,	1,	50,	1,	100,	SC_CRACKER2,	R_TEMP+0.0f +273.15f,	251,	"Rusts with salt, can be used for electrlosis of WATR", TYPE_SOLID},
    {"MORT",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	1,	-1,		SC_CRACKER2,	R_TEMP+4.0f	+273.15f,	60,		"Steam Train.", TYPE_PART},
    {"GOL ",	PIXPACK(0x0CAC00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Game Of Life! (23/3)", TYPE_SOLID},
    {"HLIF",	PIXPACK(0xFF0000),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"High Life! (like GOL 23/36)", TYPE_SOLID},
    {"ASIM",	PIXPACK(0x0000FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Assimilation! (4567/345)", TYPE_SOLID},
    {"2x2 ",	PIXPACK(0xFFFF00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"2x2! (125/36)", TYPE_SOLID},
    {"DANI",	PIXPACK(0x00FFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Day and Night! (34678/3678)", TYPE_SOLID},
    {"AMOE",	PIXPACK(0xFF00FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Amoeba! (1358/357)", TYPE_SOLID},
    {"MOVE",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"'Move' particles! Does not move things.. it is a life type(245/368)", TYPE_SOLID},
    {"PGOL",	PIXPACK(0xE05010),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Pseudo Life! (238/357)", TYPE_SOLID},
    {"DMOE",	PIXPACK(0x500000),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Diamoeba! (5678/35678)", TYPE_SOLID},
    {"34  ",	PIXPACK(0x500050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"34! (34/34)", TYPE_SOLID},
    {"LLIF",	PIXPACK(0x505050),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Long Life! (5/345)", TYPE_SOLID},
    {"STAN",	PIXPACK(0x5000FF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_LIFE,		9000.0f,				40,		"Stains! (235678/3678)", TYPE_SOLID},
    {"SPNG",  	PIXPACK(0xFFBE30),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	20, 	0,  1, 	30,	1, 	100,    SC_SOLIDS,  	R_TEMP+0.0f +273.15f,   251,    "A sponge, absorbs water.",TYPE_SOLID},
    {"RIME",  	PIXPACK(0xCCCCCC),	0.00f, 	0.00f * CFDS,   0.00f,  1.00f,   0.00f, 0.0f,   0.00f,  0.000f  * CFDS, 0, 	0, 		0, 	0, 	30,	1,  100,    SC_CRACKER2,  	243.15f,				100,    "Not quite Ice",TYPE_SOLID},
    {"FOG",  	PIXPACK(0xAAAAAA),	0.8f,	0.00f * CFDS,	0.4f,	0.70f,	-0.1f,	0.0f,	0.99f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,		SC_CRACKER2,  	243.15f,				100,    "Not quite Steam",TYPE_GAS},
    {"BCLN",	PIXPACK(0xFFD040),	0.1f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	100,	SC_SPECIAL,		R_TEMP+0.0f	+273.15f,	251,	"Breakable Clone.", TYPE_SOLID},
    {"LOVE",	PIXPACK(0xFF30FF),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_CRACKER2,	373.0f,					40,		"Love...", TYPE_SOLID},
    {"DEUT",  	PIXPACK(0x00153F),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	31,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	251,	"Deuterium oxide. Volume changes with temp, radioactive with neutrons.", TYPE_LIQUID|PROP_NEUTPENETRATE},
    {"WARP",  	PIXPACK(0x000000),	0.8f,	0.00f * CFDS,	0.9f,	0.70f,	-0.1f,	0.0f,	3.00f,	0.000f	* CFDS, 0, 	0, 		0,  0,  30, 1,  1,		SC_NUCLEAR,  	R_TEMP +273.15f,		100,    "Displaces other elements.",TYPE_GAS},
    {"PUMP",	PIXPACK(0x10103B),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	10,	1,	100,	SC_ELEC,		273.15f,				0,		"Changes pressure to its temp when activated. (use HEAT/COOL).", TYPE_SOLID},	
    {"FWRK",	PIXPACK(0x666666),	0.4f,	0.01f * CFDS,	0.99f,	0.95f,	0.0f,	0.4f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	1,	1,	97,		SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	100,	"First fireworks made, activated by heat/neutrons."},
    {"PIPE",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_SOLIDS,		273.15f,				0,		"Moves elements around, read FAQ on website for help.", TYPE_SOLID},
    {"FRZZ",	PIXPACK(0xC0E0FF),	0.7f,	0.01f * CFDS,	0.96f,	0.90f,	-0.1f,	0.05f,	0.01f,	-0.00005f* CFDS,1,	0,		0,	0,	20,	1,	50,		SC_CRACKER2,	90.0f,					46,		"FREEZE", TYPE_PART},
    {"FRZW",	PIXPACK(0x1020C0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	30,		SC_CRACKER2,	120.0f,					29,		"FREEZE WATER", TYPE_LIQUID},
    {"GRAV",	PIXPACK(0xFFE0A0),	0.7f,	0.00f * CFDS,	1.00f,	1.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Changes colour based on velocity.", TYPE_PART},    
    {"BIZR",	PIXPACK(0x00FF77),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	30,		SC_LIQUID,		R_TEMP+0.0f	+273.15f,	29,		"Bizarre... contradicts the normal state changes.", TYPE_LIQUID},
    {"BIZG",	PIXPACK(0x00FFBB),	1.0f,	0.01f * CFDS,	0.99f,	0.30f,	-0.1f,	0.0f,	2.75f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	1,		SC_CRACKER2,	R_TEMP-200.0f+273.15f,	42,		"Bizarre gas", TYPE_GAS},
    {"BIZS",	PIXPACK(0x00E455),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_CRACKER2,	R_TEMP+300.0f+273.15f,	251,	"Bizarre solid", TYPE_SOLID},    
    {"INST",	PIXPACK(0x404039),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	251,	"Instantly conducts, like walls, is now also a battery, PSCN to charge, NSCN to take.", TYPE_SOLID|PROP_CONDUCTS},    
    {"ISOZ",	PIXPACK(0xAA30D0),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	24,		SC_NUCLEAR,		R_TEMP-2.0f	+273.15f,	29,		"Radioactive liquid", TYPE_LIQUID|PROP_NEUTPENETRATE},    
    {"ISZS",	PIXPACK(0x662089),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.0007f* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_NUCLEAR,		140.00f,				251,	"Solid form of ISOZ, slowly decays.", TYPE_SOLID},        
    {"PRTI",	PIXPACK(0xEB5917),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	-0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_NUCLEAR,		R_TEMP+0.0f	+273.15f,	40,		"Portal IN.  Things go in here", TYPE_SOLID},
    {"PRTO",	PIXPACK(0x0020EB),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.005f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_NUCLEAR,		R_TEMP+0.0f	+273.15f,	40,		"Portal OUT.  Things come out here", TYPE_SOLID},
    {"PSTE",	PIXPACK(0xAA99AA),	0.6f,	0.01f * CFDS,	0.98f,	0.95f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	2,	0,		0,	0,	20,	1,	31,		SC_LIQUID,		R_TEMP-2.0f	+273.15f,	29,		"Colloid, Hardens under pressure", TYPE_LIQUID},    
    {"PSTS",	PIXPACK(0x776677),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	20,	0,	100,	SC_CRACKER,		R_TEMP-2.0f	+273.15f,	29,		"Solid form of PSTE, temporary", TYPE_SOLID},    
    {"ANAR",	PIXPACK(0xFFFFEE),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.1f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	10,		0,	0,	30,	1,	85,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	70,		"Very light dust. Behaves opposite gravity", TYPE_PART},        
    {"VINE",	PIXPACK(0x079A00),	0.0f,	0.00f * CFDS,	0.95f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	20,		0,	0,	10,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	65,		"Vine, grows", TYPE_SOLID},    
    {"INVS",	PIXPACK(0x00CCCC),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	15,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	164,	"Invisible to everything while under pressure.", TYPE_SOLID},
    {"EQVE",	PIXPACK(0xFFE0A0),	0.7f,	0.02f * CFDS,	0.96f,	0.80f,	0.0f,	0.1f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	0,	30,	0,	85,		SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	70,		"Shared velocity test", TYPE_PART},
    {"int2",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"Shield, spark it to grow", 0},
    {"int3",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"Shield, spark it to grow", 0},
    {"SHLD",	PIXPACK(0xAAAAAA),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	0,		"Shield, spark it to grow", 0},
    {"SHD2",	PIXPACK(0x777777),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 2", 0},
    {"SHD3",	PIXPACK(0x444444),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 3", 0},
    {"SHD4",	PIXPACK(0x212121),	0.0f,	0.00f * CFDS,	1.00f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	0,	100,	SC_CRACKER2,	R_TEMP+0.0f	+273.15f,	0,		"Shield lvl 4", 0},
    {"LOLZ",	PIXPACK(0x569212),	0.0f,	0.00f * CFDS,	0.00f,	0.00f,	0.0f,	0.0f,	0.0f,	0.000f	* CFDS,	0,	0,		0,	0,	0,	1,	100,	SC_CRACKER2,	373.0f,					40,		"Lolz", TYPE_SOLID},
    {"WIFI",	PIXPACK(0x40A060),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	2,	1,	100,	SC_ELEC,		R_TEMP+0.0f	+273.15f,	0,		"Wireless transmitter, color coded.", TYPE_SOLID},
    {"FILT",	PIXPACK(0x000056),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	251,	"Filter for photons, changes the color.", TYPE_SOLID},
    {"ARAY",	PIXPACK(0xFFBB00),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	1,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	251,	"Ray Emmitter. Rays create points when they collide", TYPE_SOLID},
    {"BRAY",	PIXPACK(0xFFFFFF),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	1,	1,	0,	100,	SC_ELEC,		R_TEMP+0.0f +273.15f,	251,	"Ray Point. Rays create points when they collide", TYPE_SOLID},
	{"BOOM",	PIXPACK(0xEDEB51),	0.2f,	0.15f * CFDS,	0.50f,	0.07	0.0f,	0.3f,	0.01f,	0.000f	* CFDS, 1,	5000,	2,	1,	0,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f +273.15f	251,	"Boom powder. Does not react with acid", TYPE_PART}
	//Name		Colour				Advec	Airdrag			Airloss	Loss	Collid	Grav	Diffus	Hotair			Fal	Burn	Exp	Mel	Hrd	M	Weights	Section			H						Ins		Description
};

static part_state pstates[PT_NUM] =
{
    // Name		State		Solid	 Frzp		Liquid   Mpnt		Gas		 Bpoint		'Plasma' Flashpoint
    /* NONE */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DUST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WATR */ {ST_LIQUID,	PT_ICEI, 273.15f,	PT_NONE, 0.0f,		PT_WTRV, 373.0f,	PT_NONE, 0.0f},
    /* OIL  */ {ST_LIQUID,	PT_NONE, 0.0f,  	PT_NONE, 0.0f,		PT_GAS, 333.0f,  	PT_NONE, 0.0f},
    /* FIRE */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_PLSM, 2773.0f},
    /* STNE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 983.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LAVA */ {ST_LIQUID,	PT_STNE, 973.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GUN  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* NITR */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* CLNE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GAS  */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,  	PT_NONE, 50.0f,		PT_FIRE, 573.0f},
    /* C-4  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* GOO  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* ICE  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_WATR, 274.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* METL */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SPRK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SNOW */ {ST_SOLID,	PT_NONE, 0.0f,		PT_WATR, 273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WOOD */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 873.0f},
    /* NEUT */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PLUT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PLNT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 573.0f},
    /* ACID */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* VOID */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WTRV */ {ST_GAS,		PT_RIME, 273.0f,	PT_DSTW, 371.0f,	PT_NONE, 373.0f,	PT_NONE, 0.0f},
    /* CNCT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1123.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DSTW */ {ST_LIQUID,	PT_ICEI, 273.15f,	PT_NONE, 0.0f,		PT_WTRV, 373.0f,	PT_NONE, 0.0f},
    /* SALT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1173.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SLTW */ {ST_LIQUID,	PT_ICEI, 233.0f,	PT_NONE, 0.0f,		PT_WTRV, 483.0f,	PT_NONE, 0.0f},
    /* DMND */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BMTL */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BRMT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PHOT */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* URAN */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 2373.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WAX  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_MWAX, 319.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* MWAX */ {ST_LIQUID,	PT_WAX, 318.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* PSCN */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* NSCN */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LNTG */ {ST_SOLID,	PT_NICE, 63.0f,		PT_NONE, 0.0f,		PT_NONE, 77.0f,		PT_NONE, 0.0f},
    /* FOAM */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BHOL */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WHOL */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* RBDM */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LRBD, 312.0f,	PT_NONE, 0.0f,		PT_FIRE, 961.0f},
    /* LRBD */ {ST_LIQUID,	PT_RBDM, 311.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 961.0f},
    /* HSCN */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SAND */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1973.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GLAS */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1973.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* CSCN */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1414.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BGLA */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1973.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* THDR */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PLSM */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* ETRD */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* NICE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LNTG, 63.1f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* NBLE */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BTRY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_PLSM, 2273.0f},
    /* LCRY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_BGLA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* STKM */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 620.0f},
    /* SWCH */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SMKE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 625.0f},
    /* DESL */ {ST_LIQUID,  PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 335.0f},
    /* COAL */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LO2  */ {ST_LIQUID,  PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_O2,   90.1f,		PT_NONE, 0.0f},
    /* O2   */ {ST_GAS,		PT_NONE, 0.0f,		PT_LO2,  90.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* INWR */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* YEST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_DYST, 373.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DYST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_DUST, 473.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* THRM */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GLOW */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BRCK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1223.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* HFLM */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FIRW */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FUSE */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FSEP */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* AMtr */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BCOL */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PCLN */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* HSWC */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* IRON */ {ST_SOLID,   PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* MORT */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GOL  */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* Crac */ {ST_SOLID,   PT_NONE, 0.0f, 		PT_NONE, 0.0f,  	PT_NONE, 0.0f,   	PT_FIRE, 2730.0f},
    /* RIME */ {ST_SOLID,	PT_NONE, 0.0f,		PT_WATR, 273.15f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FOG  */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BCLN */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LOVE */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WTF  */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WARP */ {ST_GAS	,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PUMP */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FWRK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FWRK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FRZZ */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FRZW */ {ST_LIQUID,	PT_ICEI, 53.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GRAV */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BIZR */ {ST_LIQUID,	PT_BIZRG, 100.0f,	PT_NONE, 0.0f,		PT_BIZRS, 400.0f,	PT_NONE, 0.0f},
    /* BIZRG*/ {ST_GAS	,	PT_BIZRG, 100.0f,	PT_BIZR , 273.0f,	PT_BIZRS, 400.0f,	PT_NONE, 0.0f},
    /* BIZRS*/ {ST_SOLID,	PT_BIZRG, 100.0f,	PT_BIZR , 273.0f,	PT_BIZRS, 400.0f,	PT_NONE, 0.0f},
    /* INST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},   
    /* ISOZ */ {ST_LIQUID,	PT_ISZS, 160.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},    
    /* ISZS */ {ST_SOLID,	PT_NONE, 0.0f,		PT_ISOZ, 300.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},   
    /* PRTI */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PRTO */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PSTE */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PSTS */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* ANAR */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* VINE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 573.0f},
    /* INVS */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* EQVE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* INST2*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* INST3*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SHLD1*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SHLD2*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SHLD3*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SHLD4*/ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LOlZ */ {ST_GAS,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WIFI */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FILT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* ARAY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BRAY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
	/* BOOM */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f}
    
};
static int grule[NGOL][9] = 
{
//   0,1,2,3,4,5,6,7,8    live=1  spawn=2 spawn&live=3
	{0,0,0,0,0,0,0,0,0},//blank
	{0,0,1,3,0,0,0,0,0},//GOL
	{0,0,1,3,0,0,2,0,0},//HLIF
	{0,0,0,2,3,3,1,1,0},//ASIM
	{0,1,1,2,0,1,2,0,0},//2x2
	{0,0,0,3,1,0,3,3,3},//DANI
	{0,1,0,3,0,3,0,2,1},//AMOE
	{0,0,1,2,1,1,2,0,2},//MOVE
	{0,0,1,3,0,2,0,2,1},//PGOL
	{0,0,0,2,0,3,3,3,3},//DMOE
	{0,0,0,3,3,0,0,0,0},//34
	{0,0,0,2,2,3,0,0,0},//LLIF
	{0,0,1,3,0,1,3,3,3},//STAN
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
int portal[8][8];
float portaltemp[8][8];
int wireless[MAX_TEMP/100][2];

extern int isplayer;
extern float player[27];

extern particle *parts;
extern particle *cb_parts;

extern unsigned char bmap[YRES/CELL][XRES/CELL];
extern unsigned char emap[YRES/CELL][XRES/CELL];

extern unsigned char cb_bmap[YRES/CELL][XRES/CELL];
extern unsigned char cb_emap[YRES/CELL][XRES/CELL];

extern int pfree;

extern unsigned pmap[YRES][XRES];
unsigned cb_pmap[YRES][XRES];

int try_move(int i, int x, int y, int nx, int ny);

void kill_part(int i);

#if defined(WIN32) && !defined(__GNUC__)
extern _inline int create_part(int p, int x, int y, int t);
#else
extern inline int create_part(int p, int x, int y, int t);
#endif

#if defined(WIN32) && !defined(__GNUC__)
extern _inline void delete_part(int x, int y);
#else
extern inline void delete_part(int x, int y);
#endif

#if defined(WIN32) && !defined(__GNUC__)
extern _inline int is_wire(int x, int y);
#else
extern inline int is_wire(int x, int y);
#endif

#if defined(WIN32) && !defined(__GNUC__)
extern _inline int is_wire_off(int x, int y);
#else
extern inline int is_wire_off(int x, int y);
#endif

void set_emap(int x, int y);

#if defined(WIN32) && !defined(__GNUC__)
_inline int parts_avg(int ci, int ni, int t);
#else
int parts_avg(int ci, int ni, int t);
#endif

int nearest_part(int ci, int t);

void update_particles_i(pixel *vid, int start, int inc);

void update_particles(pixel *vid);

void clear_area(int area_x, int area_y, int area_w, int area_h);

void create_box(int x1, int y1, int x2, int y2, int c);

int flood_parts(int x, int y, int c, int cm, int bm);

int create_parts(int x, int y, int rx, int ry, int c);

void create_line(int x1, int y1, int x2, int y2, int rx, int ry, int c);

#endif
