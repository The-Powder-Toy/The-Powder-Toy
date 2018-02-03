#ifndef SIMULATION_H
#define SIMULATION_H
#include <cstring>
#include <cstddef>
#include <vector>

#include "Config.h"
#include "Elements.h"
#include "SimulationData.h"
#include "Sign.h"
#include "Particle.h"
#include "Stickman.h"
#include "WallType.h"
#include "GOLMenu.h"
#include "MenuSection.h"
#include "elements/Element.h"

#define CHANNELS ((int)(MAX_TEMP-73)/100+2)

class Snapshot;
class SimTool;
class Brush;
class SimulationSample;
struct matrix2d;
struct vector2d;

class Simulation;
class Renderer;
class Gravity;
class Air;
class GameSave;

class Simulation
{
public:

	Gravity * grav;
	Air * air;

	std::vector<sign> signs;
	Element elements[PT_NUM];
	//Element * elements;
	std::vector<SimTool*> tools;
	unsigned int * platent;
	wall_type wtypes[UI_WALLCOUNT];
	gol_menu gmenu[NGOL];
	int goltype[NGOL];
	int grule[NGOL+1][10];
	menu_section msections[SC_TOTAL];

	int currentTick;
	int replaceModeSelected;
	int replaceModeFlags;

	char can_move[PT_NUM][PT_NUM];
	int debug_currentParticle;
	int parts_lastActiveIndex;
	int pfree;
	int NUM_PARTS;
	bool elementRecount;
	int elementCount[PT_NUM];
	int ISWIRE;
	bool force_stacking_check;
	int emp_decor;
	int emp_trigger_count;
	bool etrd_count_valid;
	int etrd_life0_count;
	int lightningRecreate;
	//Stickman
	playerst player;
	playerst player2;
	playerst fighters[MAX_FIGHTERS]; //Defined in Stickman.h
	unsigned char fighcount; //Contains the number of fighters
	bool gravWallChanged;
	//Portals and Wifi
	Particle portalp[CHANNELS][8][80];
	int portal_rx[8];
	int portal_ry[8];
	int wireless[CHANNELS][2];
	//Gol sim
	int CGOL;
	int GSPEED;
	unsigned char gol[YRES][XRES];
	unsigned short gol2[YRES][XRES][9];
	//Air sim
	float (*vx)[XRES/CELL];
	float (*vy)[XRES/CELL];
	float (*pv)[XRES/CELL];
	float (*hv)[XRES/CELL];
	//Gravity sim
	float *gravx;//gravx[(YRES/CELL) * (XRES/CELL)];
	float *gravy;//gravy[(YRES/CELL) * (XRES/CELL)];
	float *gravp;//gravp[(YRES/CELL) * (XRES/CELL)];
	float *gravmap;//gravmap[(YRES/CELL) * (XRES/CELL)];
	//Walls
	unsigned char bmap[YRES/CELL][XRES/CELL];
	unsigned char emap[YRES/CELL][XRES/CELL];
	float fvx[YRES/CELL][XRES/CELL];
	float fvy[YRES/CELL][XRES/CELL];
	//Particles
	Particle parts[NPART];
	int pmap[YRES][XRES];
	int photons[YRES][XRES];
	int pmap_count[YRES][XRES];
	//Simulation Settings
	int edgeMode;
	int gravityMode;
	int legacy_enable;
	int aheat_enable;
	int water_equal_test;
	int sys_pause;
	int framerender;
	int pretty_powder;
	int sandcolour;
	int sandcolour_frame;

	int Load(GameSave * save, bool includePressure = true);
	int Load(int x, int y, GameSave * save, bool includePressure = true);
	GameSave * Save(bool includePressure = true);
	GameSave * Save(int x1, int y1, int x2, int y2, bool includePressure = true);
	void SaveSimOptions(GameSave * gameSave);
	SimulationSample GetSample(int x, int y);

	Snapshot * CreateSnapshot();
	void Restore(const Snapshot & snap);

	int is_blocking(int t, int x, int y);
	int is_boundary(int pt, int x, int y);
	int find_next_boundary(int pt, int *x, int *y, int dm, int *em);
	int pn_junction_sprk(int x, int y, int pt);
	void photoelectric_effect(int nx, int ny);
	unsigned direction_to_map(float dx, float dy, int t);
	int do_move(int i, int x, int y, float nxf, float nyf);
	int try_move(int i, int x, int y, int nx, int ny);
	int eval_move(int pt, int nx, int ny, unsigned *rr);
	void init_can_move();
	bool IsWallBlocking(int x, int y, int type);
	bool IsValidElement(int type) {
		return (type >= 0 && type < PT_NUM && elements[type].Enabled);
	}
	void create_cherenkov_photon(int pp);
	void create_gain_photon(int pp);
	void kill_part(int i);
	bool FloodFillPmapCheck(int x, int y, int type);
	int flood_prop(int x, int y, size_t propoffset, PropertyValue propvalue, StructProperty::PropertyType proptype);
	int flood_water(int x, int y, int i, int originaly, int check);
	int FloodINST(int x, int y, int fullc, int cm);
	void detach(int i);
	void part_change_type(int i, int x, int y, int t);
	//int InCurrentBrush(int i, int j, int rx, int ry);
	//int get_brush_flags();
	int create_part(int p, int x, int y, int t, int v = -1);
	void delete_part(int x, int y);
	void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);
	int is_wire(int x, int y);
	int is_wire_off(int x, int y);
	void set_emap(int x, int y);
	int parts_avg(int ci, int ni, int t);
	void create_arc(int sx, int sy, int dx, int dy, int midpoints, int variance, int type, int flags);
	void UpdateParticles(int start, int end);
	void SimulateGoL();
	void RecalcFreeParticles(bool do_life_dec);
	void CheckStacking();
	void BeforeSim();
	void AfterSim();
	void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert);
	void clear_area(int area_x, int area_y, int area_w, int area_h);

	void SetEdgeMode(int newEdgeMode);

	//Drawing Deco
	void ApplyDecoration(int x, int y, int colR, int colG, int colB, int colA, int mode);
	void ApplyDecorationPoint(int x, int y, int colR, int colG, int colB, int colA, int mode, Brush * cBrush = NULL);
	void ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush * cBrush = NULL);
	void ApplyDecorationBox(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode);
	bool ColorCompare(Renderer *ren, int x, int y, int replaceR, int replaceG, int replaceB);
	void ApplyDecorationFill(Renderer *ren, int x, int y, int colR, int colG, int colB, int colA, int replaceR, int replaceG, int replaceB);

	//Drawing Tools like HEAT, AIR, and GRAV
	int Tool(int x, int y, int tool, int brushX, int brushY, float strength = 1.0f);
	int ToolBrush(int x, int y, int tool, Brush * cBrush, float strength = 1.0f);
	void ToolLine(int x1, int y1, int x2, int y2, int tool, Brush * cBrush, float strength = 1.0f);
	void ToolBox(int x1, int y1, int x2, int y2, int tool, float strength = 1.0f);
	
	//Drawing Walls
	int CreateWalls(int x, int y, int rx, int ry, int wall, Brush * cBrush = NULL);
	void CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall, Brush * cBrush = NULL);
	void CreateWallBox(int x1, int y1, int x2, int y2, int wall);
	int FloodWalls(int x, int y, int wall, int bm);

	//Drawing Particles
	int CreateParts(int positionX, int positionY, int c, Brush * cBrush, int flags = -1);
	int CreateParts(int x, int y, int rx, int ry, int c, int flags = -1);
	int CreatePartFlags(int x, int y, int c, int flags);
	void CreateLine(int x1, int y1, int x2, int y2, int c, Brush * cBrush, int flags = -1);
	void CreateLine(int x1, int y1, int x2, int y2, int c);
	void CreateBox(int x1, int y1, int x2, int y2, int c, int flags = -1);
	int FloodParts(int x, int y, int c, int cm, int flags = -1);

	
	void GetGravityField(int x, int y, float particleGrav, float newtonGrav, float & pGravX, float & pGravY);

	int GetParticleType(std::string type);

	void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[]);
	void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[]);
	int get_wavelength_bin(int *wm);
	int get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny);
	int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny);
	void clear_sim();
	Simulation();
	~Simulation();

	bool InBounds(int x, int y)
	{
		return (x>=0 && y>=0 && x<XRES && y<YRES);
	}

	// Element IDs can be stored in other properties
	// These functions return true if an element stores a particle type in a property
	static bool TypeInCtype(int el);
	static bool TypeInTmp(int el);
	static bool TypeInTmp2(int el);

	// These don't really belong anywhere at the moment, so go here for loop edge mode
	static int remainder_p(int x, int y)
	{
		return (x % y) + (x>=0 ? 0 : y);
	}
	static float remainder_p(float x, float y)
	{
		return std::fmod(x, y) + (x>=0 ? 0 : y);
	}
};

#endif /* SIMULATION_H */
