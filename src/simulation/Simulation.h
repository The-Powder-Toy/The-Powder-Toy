/*
 * Simulation.h
 *
 *  Created on: Jan 2, 2012
 *      Author: Simon
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_
#include <cstring>
#include <cstddef>
#include <vector>

#include "Config.h"
#include "Elements.h"
#include "SimulationData.h"
#include "Sign.h"
#include "Particle.h"
#include "Player.h"
#include "WallType.h"
#include "GOLMenu.h"
#include "MenuSection.h"

#define CHANNELS ((int)(MAX_TEMP-73)/100+2)

class Element;
class SimTool;
class Brush;
struct SimulationSample;
struct matrix2d;
struct vector2d;

class Simulation;
class Renderer;
class Gravity;
class Air;
class GameSave;

//#ifdef _cplusplus
class Simulation
{
private:
public:

	Gravity * grav;
	Air * air;

	std::vector<sign> signs;
	Element * elements;
	std::vector<SimTool*> tools;
	unsigned int * platent;
	wall_type wtypes[UI_WALLCOUNT];
	gol_menu gmenu[NGOL];
	int goltype[NGOL];
	int grule[NGOL+1][10];
	menu_section msections[SC_TOTAL];

	int currentTick;

	playerst player;
	playerst player2;
	playerst fighters[256]; //255 is the maximum number of fighters
	unsigned char fighcount; //Contains the number of fighters
	int lighting_recreate;
	bool gravWallChanged;
	Particle portalp[CHANNELS][8][80];
	Particle emptyparticle;
	int portal_rx[8];
	int portal_ry[8];
	int wireless[CHANNELS][2];
	char can_move[PT_NUM][PT_NUM];
	int parts_lastActiveIndex;// = NPART-1;
	int pfree;
	int NUM_PARTS;
	int elementCount[PT_NUM];
	int ISWIRE;
	int force_stacking_check;
	//Gol sim
	int CGOL;
	int ISGOL;
	int GSPEED;
	unsigned char gol[YRES][XRES];
	unsigned char gol2[YRES][XRES][NGOL+1];
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
	//
	int edgeMode;
	int gravityMode;
	//int airMode;
	int legacy_enable;
	int aheat_enable;
	int VINE_MODE;
	int water_equal_test;
	int sys_pause;
	int framerender;
	int pretty_powder;
	//
	int sandcolour_r;
	int sandcolour_g;
	int sandcolour_b; //TODO: Make a single variable

	int Load(GameSave * save);
	int Load(int x, int y, GameSave * save);
	GameSave * Save();
	GameSave * Save(int x1, int y1, int x2, int y2);
	SimulationSample Get(int x, int y);
	inline int is_blocking(int t, int x, int y);
	inline int is_boundary(int pt, int x, int y);
	inline int find_next_boundary(int pt, int *x, int *y, int dm, int *em);
	inline int pn_junction_sprk(int x, int y, int pt);
	inline void photoelectric_effect(int nx, int ny);
	inline unsigned direction_to_map(float dx, float dy, int t);
	inline int do_move(int i, int x, int y, float nxf, float nyf);
	inline int try_move(int i, int x, int y, int nx, int ny);
	inline int eval_move(int pt, int nx, int ny, unsigned *rr);
	void init_can_move();
	void create_cherenkov_photon(int pp);
	void create_gain_photon(int pp);
	inline void kill_part(int i);
	int flood_prop(int x, int y, size_t propoffset, void * propvalue, StructProperty::PropertyType proptype);
	int flood_prop_2(int x, int y, size_t propoffset, void * propvalue, StructProperty::PropertyType proptype, int parttype, char * bitmap);
	int flood_water(int x, int y, int i, int originaly, int check);
	inline void detach(int i);
	inline void part_change_type(int i, int x, int y, int t);
	inline int create_part_add_props(int p, int x, int y, int tv, int rx, int ry);
	//int InCurrentBrush(int i, int j, int rx, int ry);
	//int get_brush_flags();
	inline int create_part(int p, int x, int y, int t);
	inline void delete_part(int x, int y, int flags);
	void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);
	inline int is_wire(int x, int y);
	inline int is_wire_off(int x, int y);
	inline void set_emap(int x, int y);
	inline int parts_avg(int ci, int ni, int t);
	void create_arc(int sx, int sy, int dx, int dy, int midpoints, int variance, int type, int flags);
	int nearest_part(int ci, int t, int max_d);
	void update_particles_i(int start, int inc);
	void update_particles();
	void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert);
	void clear_area(int area_x, int area_y, int area_w, int area_h);

	void SetEdgeMode(int newEdgeMode);

	int Tool(int x, int y, int tool, float strength = 1.0f);
	int ToolBrush(int x, int y, int tool, Brush * cBrush, float strength = 1.0f);
	void ToolLine(int x1, int y1, int x2, int y2, int tool, Brush * cBrush, float strength = 1.0f);
	void ToolBox(int x1, int y1, int x2, int y2, int tool, Brush * cBrush, float strength = 1.0f);
	
	void CreateBox(int x1, int y1, int x2, int y2, int c, int flags);
	int FloodINST(int x, int y, int fullc, int cm);
	int FloodParts(int x, int y, int c, int cm, int bm, int flags);
	//Create particles from brush/mask
	int CreateParts(int positionX, int positionY, int c, Brush * cBrush);
	//Old particle creation, will create a crappy square, do not use
	int CreateParts(int x, int y, int rx, int ry, int c, int flags);
	void CreateLine(int x1, int y1, int x2, int y2, int c, Brush * cBrush);
	void CreateLine(int x1, int y1, int x2, int y2, int rx, int ry, int c, int flags);
	
	void CreateWallBox(int x1, int y1, int x2, int y2, int c, int flags);
	int FloodWalls(int x, int y, int c, int cm, int bm, int flags);
	int CreateWalls(int x, int y, int rx, int ry, int c, int flags, Brush * cBrush = NULL);
	void CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int c, int flags, Brush * cBrush = NULL);
	
	void ApplyDecoration(int x, int y, int colR, int colG, int colB, int colA, int mode);
	void ApplyDecorationPoint(int x, int y, int colR, int colG, int colB, int colA, int mode, Brush * cBrush = NULL);
	void ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush * cBrush = NULL);
	void ApplyDecorationBox(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode);
	
	void GetGravityField(int x, int y, float particleGrav, float newtonGrav, float & pGravX, float & pGravY);

	int GetParticleType(std::string type);

	void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate);
	inline void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[]);
	inline void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[]);
	inline int get_wavelength_bin(int *wm);
	inline int get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny);
	inline int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny);
	void clear_sim();
	void UpdateParticles();
	Simulation();
	~Simulation();
};
//#endif

#endif /* SIMULATION_H_ */
