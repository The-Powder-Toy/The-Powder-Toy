#ifndef POWDER_H
#define POWDER_H

#include "graphics.h"
#include "defines.h"
#include "interface.h"

#define CM_COUNT 7
#define CM_FANCY 6
#define CM_HEAT 5
#define CM_BLOB 4
#define CM_FIRE 3
#define CM_PERS 2
#define CM_PRESS 1
#define CM_VEL 0

#define UI_WALLSTART 37
#define UI_WALLCOUNT 19

#define SPC_AIR 136
#define SPC_HEAT 137
#define SPC_COOL 138
#define SPC_VACUUM 139

#define WL_WALLELEC	22
#define WL_EWALL	23
#define WL_DETECT	24
#define WL_STREAM	25
#define WL_SIGN	26
#define WL_FAN	27
#define WL_ALLOWLIQUID	28
#define WL_DESTROYALL	29
#define WL_ERASE	30
#define WL_WALL	31
#define WL_ALLOWAIR	32
#define WL_ALLOWSOLID	33
#define WL_ALLOWALLELEC	34
#define WL_EHOLE	35
#define WL_ALLOWGAS	40

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
#define PT_NUM  78

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
#define TYPE_ENERGY			0x0018 //16 Energy (Thunder, Light, Neutrons etc.)
#define PROP_CONDUCTS		0x0020 //32 Conducts electricity (Currently Unused)
#define PROP_BLACK			0x0040 //64 Absorbs Photons
#define PROP_NEUTPENETRATE	0x0080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x0100 //256 Absorbs neutrons, reflect is default
#define PROP_DEADLY			0x0200 //512 Is deadly for stickman.
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
    //Name		Colour			Advec	Airdrag		Airloss	Loss	Collid	Grav	Diffus	Hotair		Fal	Burn	Exp	Mel	Hrd 	M	Weight	Section			H				Ins(real world, by triclops200) Description
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
    {"C-4",		PIXPACK(0xD080E0),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	1000,	2,	50,	1,	1,	100,	SC_EXPLOSIVE,	R_TEMP+0.0f	+273.15f,	88,		"Solid. Pressure sensitive explosive.", TYPE_SOLID},
    {"GOO",		PIXPACK(0x804000),	0.1f,	0.00f * CFDS,	0.97f,	0.50f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	12,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	75,		"Solid. Deforms and disappears under pressure.", TYPE_SOLID},
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
    {"CNCT",	PIXPACK(0xC0C0C0),	0.4f,	0.04f * CFDS,	0.94f,	0.95f,	-0.1f,	0.3f,	0.00f,	0.000f	* CFDS,	1,	0,		0,	2,	25,	1,	55,		SC_POWDERS,		R_TEMP+0.0f	+273.15f,	100,	"Concrete, stronger than stone.", TYPE_PART},
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
    {"GLAS",	PIXPACK(0x404040),	0.0f,	0.00f * CFDS,	0.90f,	0.00f,	0.0f,	0.0f,	0.00f,	0.000f	* CFDS,	0,	0,		0,	0,	1,	1,	100,	SC_SOLIDS,		R_TEMP+0.0f	+273.15f,	150,	"Solid. Meltable. Shatters under pressure", TYPE_SOLID},
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
    {"IRON",	PIXPACK(0x707070),	0.0f,	0.00f * CFDS,	0.90f,  0.00f,  0.0f,	0.0f,	0.00f,  0.000f	* CFDS, 0,	0,		0,	1,	50,	0,	100,	SC_SOLIDS,		R_TEMP+0.0f +273.15f,	251,	"Rusts with salt, can be used for electrlosis of WATR", TYPE_SOLID},
    {"MORT",	PIXPACK(0xE0E0E0),	0.0f,	0.00f * CFDS,	1.00f,	1.00f,	-0.99f,	0.0f,	0.01f,	0.002f	* CFDS,	0,	0,		0,	0,	0,	0,	-1,		SC_NUCLEAR,		R_TEMP+4.0f	+273.15f,	60,		"Steam Train.", TYPE_PART},
    //Name		Colour			Advec	Airdrag		Airloss	Loss	Collid	Grav	Diffus	Hotair		Fal	Burn	Exp	Mel	Hrd	M	Weights	Section			H				Ins(real world, by triclops200) Description
};

static part_state pstates[PT_NUM] =
{
    // Name			Solid	 Frzp		Liquid   Mpnt		Gas	   Bpoint
    /* NONE */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DUST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WATR */ {ST_LIQUID,	PT_ICEI, 273.15f,	PT_NONE, 0.0f,		PT_WTRV, 373.0f,	PT_NONE, 0.0f},
    /* OIL  */ {ST_LIQUID,	PT_NONE, 0.0f,  	PT_NONE, 0.0f,		PT_GAS, 333.0f,  	PT_NONE, 0.0f},
    /* FIRE */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_PLSM, 2773.0f},
    /* STNE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 983.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LAVA */ {ST_LIQUID,	PT_STNE, 973.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GUN  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* NITR */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* CLNE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GAS  */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,  	PT_NONE, 50.0f,		PT_FIRE, 573.0f},
    /* C-4  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 673.0f},
    /* GOO  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* ICE  */ {ST_SOLID,	PT_NONE, 0.0f,		PT_WATR, 274.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* METL */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SPRK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SNOW */ {ST_SOLID,	PT_NONE, 0.0f,		PT_WATR, 273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WOOD */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 873.0f},
    /* NEUT */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PLUT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PLNT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 573.0f},
    /* ACID */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* VOID */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* WTRV */ {ST_GAS,	PT_ICEI, 273.0f,	PT_DSTW, 371.0f,	PT_NONE, 373.0f,	PT_NONE, 0.0f},
    /* CNCT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1123.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DSTW */ {ST_LIQUID,	PT_ICEI, 273.15f,	PT_NONE, 0.0f,		PT_WTRV, 373.0f,	PT_NONE, 0.0f},
    /* SALT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1173.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SLTW */ {ST_LIQUID,	PT_ICEI, 233.0f,	PT_NONE, 0.0f,		PT_WTRV, 483.0f,	PT_NONE, 0.0f},
    /* DMND */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BMTL */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BRMT */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PHOT */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
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
    /* NBLE */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BTRY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_PLSM, 2273.0f},
    /* LCRY */ {ST_SOLID,	PT_NONE, 0.0f,		PT_BGLA, 1273.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* STKM */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 620.0f},
    /* SWCH */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* SMKE */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 625.0f},
    /* DESL */ {ST_LIQUID,  PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_FIRE, 335.0f},
    /* COAL */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* LO2  */ {ST_LIQUID,  PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_O2,   90.1f,		PT_NONE, 0.0f},
    /* O2   */ {ST_GAS,	PT_NONE, 0.0f,		PT_LO2,  90.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* INWR */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* YEST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_DYST, 373.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* DYST */ {ST_SOLID,	PT_NONE, 0.0f,		PT_DUST, 473.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* THRM */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* GLOW */ {ST_LIQUID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BRCK */ {ST_SOLID,	PT_NONE, 0.0f,		PT_LAVA, 1223.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* HFLM */ {ST_GAS,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FIRW */ {ST_SOLID,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FUSE */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* FSEP */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* AMtr */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* BCOL */ {ST_SOLID,   PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* PCLN */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* HSWC */ {ST_NONE,	PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f,		PT_NONE, 0.0f},
    /* IRON */ {ST_SOLID,   PT_NONE, 0.0f,		PT_LAVA, 1687.0f,	PT_NONE, 0.0f,		PT_NONE, 0.0f},
};

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

#ifdef WIN32
extern _inline int create_part(int p, int x, int y, int t);
#else
extern inline int create_part(int p, int x, int y, int t);
#endif

#ifdef WIN32
extern _inline void delete_part(int x, int y);
#else
extern inline void delete_part(int x, int y);
#endif

#ifdef WIN32
extern _inline int is_wire(int x, int y);
#else
extern inline int is_wire(int x, int y);
#endif

#ifdef WIN32
extern _inline int is_wire_off(int x, int y);
#else
extern inline int is_wire_off(int x, int y);
#endif

void set_emap(int x, int y);

#ifdef WIN32
_inline int parts_avg(int ci, int ni);
#else
int parts_avg(int ci, int ni);
#endif

int nearest_part(int ci, int t);

void update_particles_i(pixel *vid, int start, int inc);

void update_particles(pixel *vid);

void clear_area(int area_x, int area_y, int area_w, int area_h);

void create_box(int x1, int y1, int x2, int y2, int c);

int flood_parts(int x, int y, int c, int cm, int bm);

int create_parts(int x, int y, int r, int c);

void create_line(int x1, int y1, int x2, int y2, int r, int c);

#endif
