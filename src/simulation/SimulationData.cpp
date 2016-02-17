#include "SimulationData.h"
#include "Lang.h"
//#include "ElementFunctions.h"
#include "ElementGraphics.h"
#include "Elements.h"

gol_menu * LoadGOLMenu(int & golMenuCount)
{
	gol_menu golMenu[NGOL] =
	{
		{"GOL",		PIXPACK(0x0CAC00), 0, TEXT_GOL_GOL_DESC},
		{"HLIF",	PIXPACK(0xFF0000), 1, TEXT_GOL_HLIF_DESC},
		{"ASIM",	PIXPACK(0x0000FF), 2, TEXT_GOL_ASIM_DESC},
		{"2x2",		PIXPACK(0xFFFF00), 3, TEXT_GOL_2X2_DESC},
		{"DANI",	PIXPACK(0x00FFFF), 4, TEXT_GOL_DANI_DESC},
		{"AMOE",	PIXPACK(0xFF00FF), 5, TEXT_GOL_AMOE_DESC},
		{"MOVE",	PIXPACK(0xFFFFFF), 6, TEXT_GOL_MOVE_DESC},
		{"PGOL",	PIXPACK(0xE05010), 7, TEXT_GOL_PGOL_DESC},
		{"DMOE",	PIXPACK(0x500000), 8, TEXT_GOL_DMOE_DESC},
		{"34",		PIXPACK(0x500050), 9, TEXT_GOL_34_DESC},
		{"LLIF",	PIXPACK(0x505050), 10, TEXT_GOL_LLIF_DESC},
		{"STAN",	PIXPACK(0x5000FF), 11, TEXT_GOL_STAN_DESC},
		{"SEED",	PIXPACK(0xFBEC7D), 12, TEXT_GOL_SEED_DESC},
		{"MAZE",	PIXPACK(0xA8E4A0), 13, TEXT_GOL_MAZE_DESC},
		{"COAG",	PIXPACK(0x9ACD32), 14, TEXT_GOL_COAG_DESC},
		{"WALL",	PIXPACK(0x0047AB), 15, TEXT_GOL_WALL_DESC},
		{"GNAR",	PIXPACK(0xE5B73B), 16, TEXT_GOL_GNAR_DESC},
		{"REPL",	PIXPACK(0x259588), 17, TEXT_GOL_REPL_DESC},
		{"MYST",	PIXPACK(0x0C3C00), 18, TEXT_GOL_MYST_DESC},
		{"LOTE",	PIXPACK(0xFF0000), 19, TEXT_GOL_LOTE_DESC},
		{"FRG2",	PIXPACK(0x00FF00), 20, TEXT_GOL_FRG2_DESC},
		{"STAR",	PIXPACK(0x0000FF), 21, TEXT_GOL_STAR_DESC},
		{"FROG",	PIXPACK(0x00AA00), 22, TEXT_GOL_FROG_DESC},
		{"BRAN",	PIXPACK(0xCCCC00), 23, TEXT_GOL_BRAN_DESC}
	};
	golMenuCount = NGOL;
	gol_menu * golMenuT = (gol_menu*)malloc(NGOL*sizeof(gol_menu));
	memcpy(golMenuT, golMenu, NGOL*sizeof(gol_menu));
	return golMenuT;
}

int * LoadGOLRules(int & golRuleCount)
{
	int golRules[NGOL+1][10] =
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
	golRuleCount = NGOL+1;
	int * golRulesT = (int*)malloc((golRuleCount*10)*sizeof(int));
	memcpy(golRulesT, golRules, (golRuleCount*10)*sizeof(int));
	return golRulesT;
}

int * LoadGOLTypes(int & golTypeCount)
{
	int golTypes[NGOL] =
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
	golTypeCount = NGOL;
	int * golTypesT = (int*)malloc((golTypeCount)*sizeof(int));
	memcpy(golTypesT, golTypes, (golTypeCount)*sizeof(int));
	return golTypesT;
}

wall_type * LoadWalls(int & wallCount)
{
	wall_type wtypes[] =
	{
		{PIXPACK(0x808080), PIXPACK(0x000000), 0, Renderer::WallIcon, TEXT_WALL_ERASE_NAME,	"DEFAULT_WL_ERASE",	TEXT_WALL_ERASE_DESC},
		{PIXPACK(0xC0C0C0), PIXPACK(0x101010), 0, Renderer::WallIcon, TEXT_WALL_CNDTW_NAME,	"DEFAULT_WL_CNDTW",	TEXT_WALL_CNDTW_DESC},
		{PIXPACK(0x808080), PIXPACK(0x808080), 0, Renderer::WallIcon, TEXT_WALL_EWALL_NAME,	"DEFAULT_WL_EWALL",	TEXT_WALL_EWALL_DESC},
		{PIXPACK(0xFF8080), PIXPACK(0xFF2008), 1, Renderer::WallIcon, TEXT_WALL_DTECT_NAME,	"DEFAULT_WL_DTECT",	TEXT_WALL_DTECT_DESC},
		{PIXPACK(0x808080), PIXPACK(0x000000), 0, Renderer::WallIcon, TEXT_WALL_STRM_NAME,	"DEFAULT_WL_STRM",	TEXT_WALL_STRM_DESC},
		{PIXPACK(0x8080FF), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_FAN_NAME,	"DEFAULT_WL_FAN",	TEXT_WALL_FAN_DESC},
		{PIXPACK(0xC0C0C0), PIXPACK(0x101010), 2, Renderer::WallIcon, TEXT_WALL_LIQD_NAME,	"DEFAULT_WL_LIQD",	TEXT_WALL_LIQD_DESC},
		{PIXPACK(0x808080), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_ABSRB_NAME,	"DEFAULT_WL_ABSRB",	TEXT_WALL_ABSRB_DESC},
		{PIXPACK(0x808080), PIXPACK(0x000000), 3, Renderer::WallIcon, TEXT_WALL_WALL_NAME,	"DEFAULT_WL_WALL",	TEXT_WALL_WALL_DESC},
		{PIXPACK(0x3C3C3C), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_AIR_NAME,	"DEFAULT_WL_AIR",	TEXT_WALL_AIR_DESC},
		{PIXPACK(0x575757), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_POWDR_NAME,	"DEFAULT_WL_POWDR",	TEXT_WALL_POWDR_DESC},
		{PIXPACK(0xFFFF22), PIXPACK(0x101010), 2, Renderer::WallIcon, TEXT_WALL_CNDTR_NAME,	"DEFAULT_WL_CNDTR",	TEXT_WALL_CNDTR_DESC},
		{PIXPACK(0x242424), PIXPACK(0x101010), 0, Renderer::WallIcon, TEXT_WALL_EHOLE_NAME,	"DEFAULT_WL_EHOLE",	TEXT_WALL_EHOLE_DESC},
		{PIXPACK(0x579777), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_GAS_NAME,	"DEFAULT_WL_GAS",	TEXT_WALL_GAS_DESC},
		{PIXPACK(0xFFEE00), PIXPACK(0xAA9900), 4, Renderer::WallIcon, TEXT_WALL_GRVTY_NAME,	"DEFAULT_WL_GRVTY",	TEXT_WALL_GRVTY_DESC},
		{PIXPACK(0xFFAA00), PIXPACK(0xAA5500), 4, Renderer::WallIcon, TEXT_WALL_ENRGY_NAME,	"DEFAULT_WL_ENRGY",	TEXT_WALL_ENRGY_DESC},
		{PIXPACK(0xDCDCDC), PIXPACK(0x000000), 1, Renderer::WallIcon, TEXT_WALL_NOAIR_NAME,	"DEFAULT_WL_NOAIR",	TEXT_WALL_NOAIR_DESC},
		{PIXPACK(0x808080), PIXPACK(0x000000), 0, Renderer::WallIcon, TEXT_WALL_ERASEA_NAME,"DEFAULT_WL_ERASEA",TEXT_WALL_ERASEA_DESC},
	};
	wallCount = UI_WALLCOUNT;
	wall_type * wtypesT = (wall_type*)malloc(UI_WALLCOUNT*sizeof(wall_type));
	memcpy(wtypesT, wtypes, UI_WALLCOUNT*sizeof(wall_type));
	return wtypesT;
}

menu_section * LoadMenus(int & menuCount)
{
	menu_section msections[] = //doshow does not do anything currently.
	{
		{L"\xC1", TEXT_MENU_NAME_WALL, 0, 1},
		{L"\xC2", TEXT_MENU_NAME_ELEC, 0, 1},
		{L"\xD6", TEXT_MENU_NAME_POWER, 0, 1},
		{L"\x99", TEXT_MENU_NAME_SENSOR, 0, 1},
		{L"\xE2", TEXT_MENU_NAME_FORCE, 0, 1},
		{L"\xC3", TEXT_MENU_NAME_EXPLD, 0, 1},
		{L"\xC5", TEXT_MENU_NAME_GAS, 0, 1},
		{L"\xC4", TEXT_MENU_NAME_LIQUID, 0, 1},
		{L"\xD0", TEXT_MENU_NAME_POWDER, 0, 1},
		{L"\xD1", TEXT_MENU_NAME_SOLID, 0, 1},
		{L"\xC6", TEXT_MENU_NAME_RADIO, 0, 1},
		{L"\xCC", TEXT_MENU_NAME_SPEC, 0, 1},
		{L"\xD2", TEXT_MENU_NAME_GOL, 0, 1},
		{L"\xD7", TEXT_MENU_NAME_TOOL, 0, 1},
		{L"\xE4", TEXT_MENU_NAME_DECO, 0, 1},
		{L"\xC8", TEXT_MENU_NAME_CRACK1, 0, 0},
		{L"\xC8", TEXT_MENU_NAME_CRACK2, 0, 0},
	};
	menuCount = SC_TOTAL;
	menu_section * msectionsT = (menu_section*)malloc(SC_TOTAL*sizeof(menu_section));
	memcpy(msectionsT, msections, SC_TOTAL*sizeof(menu_section));
	return msectionsT;
}

unsigned int * LoadLatent(int & elementCount)
{
	unsigned int platent[PT_NUM] =
	{
		/* NONE */ 0,
		/* DUST */ 0,
		/* WATR */ 7500,
		/* OIL  */ 0,
		/* FIRE */ 0,
		/* STNE */ 0,
		/* LAVA */ 0,
		/* GUN  */ 0,
		/* NITR */ 0,
		/* CLNE */ 0,
		/* GAS  */ 0,
		/* C-4  */ 0,
		/* GOO  */ 0,
		/* ICE  */ 1095,
		/* METL */ 919,
		/* SPRK */ 0,
		/* SNOW */ 1095,
		/* WOOD */ 0,
		/* NEUT */ 0,
		/* PLUT */ 0,
		/* PLNT */ 0,
		/* ACID */ 0,
		/* VOID */ 0,
		/* WTRV */ 0,
		/* CNCT */ 0,
		/* DSTW */ 7500,
		/* SALT */ 0,
		/* SLTW */ 7500,
		/* DMND */ 0,
		/* BMTL */ 0,
		/* BRMT */ 0,
		/* PHOT */ 0,
		/* URAN */ 0,
		/* WAX  */ 0,
		/* MWAX */ 0,
		/* PSCN */ 0,
		/* NSCN */ 0,
		/* LN2  */ 0,
		/* INSL */ 0,
		/* VACU */ 0,
		/* VENT */ 0,
		/* RBDM */ 0,
		/* LRBD */ 0,
		/* NTCT */ 0,
		/* SAND */ 0,
		/* GLAS */ 0,
		/* PTCT */ 0,
		/* BGLA */ 0,
		/* THDR */ 0,
		/* PLSM */ 0,
		/* ETRD */ 0,
		/* NICE */ 0,
		/* NBLE */ 0,
		/* BTRY */ 0,
		/* LCRY */ 0,
		/* STKM */ 0,
		/* SWCH */ 0,
		/* SMKE */ 0,
		/* DESL */ 0,
		/* COAL */ 0,
		/* LO2  */ 0,
		/* O2   */ 0,
		/* INWR */ 0,
		/* YEST */ 0,
		/* DYST */ 0,
		/* THRM */ 0,
		/* GLOW */ 0,
		/* BRCK */ 0,
		/* CFLM */ 0,
		/* FIRW */ 0,
		/* FUSE */ 0,
		/* FSEP */ 0,
		/* AMTR */ 0,
		/* BCOL */ 0,
		/* PCLN */ 0,
		/* HSWC */ 0,
		/* IRON */ 0,
		/* MORT */ 0,
		/* LIFE */ 0,
		/* DLAY */ 0,
		/* CO2  */ 0,
		/* DRIC */ 0,
		/* CBNW */ 7500,
		/* STOR */ 0,
		/* STOR */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* SPNG */ 0,
		/* RIME */ 0,
		/* FOG  */ 0,
		/* BCLN */ 0,
		/* LOVE */ 0,
		/* DEUT */ 0,
		/* WARP */ 0,
		/* PUMP */ 0,
		/* FWRK */ 0,
		/* PIPE */ 0,
		/* FRZZ */ 0,
		/* FRZW */ 0,
		/* GRAV */ 0,
		/* BIZR */ 0,
		/* BIZRG*/ 0,
		/* BIZRS*/ 0,
		/* INST */ 0,
		/* ISOZ */ 0,
		/* ISZS */ 0,
		/* PRTI */ 0,
		/* PRTO */ 0,
		/* PSTE */ 0,
		/* PSTS */ 0,
		/* ANAR */ 0,
		/* VINE */ 0,
		/* INVS */ 0,
		/* EQVE */ 0,
		/* SPWN2*/ 0,
		/* SPAWN*/ 0,
		/* SHLD1*/ 0,
		/* SHLD2*/ 0,
		/* SHLD3*/ 0,
		/* SHLD4*/ 0,
		/* LOlZ */ 0,
		/* WIFI */ 0,
		/* FILT */ 0,
		/* ARAY */ 0,
		/* BRAY */ 0,
		/* STKM2*/ 0,
		/* BOMB */ 0,
		/* C-5  */ 0,
		/* SING */ 0,
		/* QRTZ */ 0,
		/* PQRT */ 0,
		/* EMP  */ 0,
		/* BREL */ 0,
		/* ELEC */ 0,
		/* ACEL */ 0,
		/* DCEL */ 0,
		/* TNT  */ 0,
		/* IGNP */ 0,
		/* BOYL */ 0,
		/* GEL  */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* FREE */ 0,
		/* WIND */ 0,
		/* H2   */ 0,
		/* SOAP */ 0,
		/* NBHL */ 0,
		/* NWHL */ 0,
		/* MERC */ 0,
		/* PBCN */ 0,
		/* GPMP */ 0,
		/* CLST */ 0,
		/* WIRE */ 0,
		/* GBMB */ 0,
		/* FIGH */ 0,
		/* FRAY */ 0,
		/* REPL */ 0,
	};
	elementCount = PT_NUM;
	unsigned int * platentT = (unsigned int*)malloc(PT_NUM*sizeof(unsigned int));
	memcpy(platentT, platent, PT_NUM*sizeof(unsigned int));
	return platentT;
}
