#pragma once
#include "Particle.h"
#include "Stickman.h"
#include "WallType.h"
#include "Sign.h"
#include "ElementDefs.h"
#include "BuiltinGOL.h"
#include "MenuSection.h"
#include "CoordStack.h"
#include "gravity/GravityPtr.h"
#include "Element.h"
#include "SimulationConfig.h"
#include <cstring>
#include <cstddef>
#include <vector>
#include <array>
#include <memory>
#include <optional>

constexpr int CHANNELS = int(MAX_TEMP - 73) / 100 + 2;

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

	GravityPtr grav;
	Air * air;

	std::vector<sign> signs;
	std::array<Element, PT_NUM> elements;
	//Element * elements;
	std::vector<SimTool> tools;
	std::vector<unsigned int> platent;
	std::vector<wall_type> wtypes;
	std::vector<menu_section> msections;

	int currentTick;
	int replaceModeSelected;
	int replaceModeFlags;

	char can_move[PT_NUM][PT_NUM];
	int debug_nextToUpdate;
	int debug_mostRecentlyUpdated = -1; // -1 when between full update loops
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
	unsigned int gol[YRES][XRES][5];
	//Air sim
	float (*vx)[XCELLS];
	float (*vy)[XCELLS];
	float (*pv)[XCELLS];
	float (*hv)[XCELLS];
	//Gravity sim
	float *gravx;//gravx[YCELLS * XCELLS];
	float *gravy;//gravy[YCELLS * XCELLS];
	float *gravp;//gravp[YCELLS * XCELLS];
	float *gravmap;//gravmap[YCELLS * XCELLS];
	//Walls
	unsigned char bmap[YCELLS][XCELLS];
	unsigned char emap[YCELLS][XCELLS];
	float fvx[YCELLS][XCELLS];
	float fvy[YCELLS][XCELLS];
	//Particles
	Particle parts[NPART];
	int pmap[YRES][XRES];
	int photons[YRES][XRES];
	unsigned int pmap_count[YRES][XRES];
	//Simulation Settings
	int edgeMode;
	int gravityMode;
	float customGravityX;
	float customGravityY;
	int legacy_enable;
	int aheat_enable;
	int water_equal_test;
	int sys_pause;
	int framerender;
	int pretty_powder;
	int sandcolour;
	int sandcolour_frame;
	int deco_space;

	int Load(const GameSave * save, bool includePressure);
	int Load(const GameSave * save, bool includePressure, int x, int y);
	GameSave * Save(bool includePressure);
	GameSave * Save(bool includePressure, int x1, int y1, int x2, int y2);
	void SaveSimOptions(GameSave * gameSave);
	SimulationSample GetSample(int x, int y);

	std::unique_ptr<Snapshot> CreateSnapshot();
	void Restore(const Snapshot &snap);

	int is_blocking(int t, int x, int y);
	int is_boundary(int pt, int x, int y);
	int find_next_boundary(int pt, int *x, int *y, int dm, int *em, bool reverse);
	void photoelectric_effect(int nx, int ny);
	unsigned direction_to_map(float dx, float dy, int t);
	int do_move(int i, int x, int y, float nxf, float nyf);
	bool move(int i, int x, int y, float nxf, float nyf);
	int try_move(int i, int x, int y, int nx, int ny);
	int eval_move(int pt, int nx, int ny, unsigned *rr);
	void init_can_move();
	bool IsWallBlocking(int x, int y, int type);
	bool IsElement(int type) const {
		return (type > 0 && type < PT_NUM && elements[type].Enabled);
	}
	bool IsElementOrNone(int type) const {
		return (type >= 0 && type < PT_NUM && elements[type].Enabled);
	}
	void create_cherenkov_photon(int pp);
	void create_gain_photon(int pp);
	void kill_part(int i);
	bool FloodFillPmapCheck(int x, int y, int type);
	int flood_prop(int x, int y, size_t propoffset, PropertyValue propvalue, StructProperty::PropertyType proptype);
	bool flood_water(int x, int y, int i);
	int FloodINST(int x, int y);
	void detach(int i);
	bool part_change_type(int i, int x, int y, int t);
	//int InCurrentBrush(int i, int j, int rx, int ry);
	//int get_brush_flags();
	int create_part(int p, int x, int y, int t, int v = -1);
	void delete_part(int x, int y);
	void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);
	int is_wire(int x, int y);
	int is_wire_off(int x, int y);
	void set_emap(int x, int y);
	int parts_avg(int ci, int ni, int t);
	void UpdateParticles(int start, int end); // Dispatches an update to the range [start, end).
	void SimulateGoL();
	void RecalcFreeParticles(bool do_life_dec);
	void CheckStacking();
	void BeforeSim();
	void AfterSim();
	void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert);
	void clear_area(int area_x, int area_y, int area_w, int area_h);

	void SetEdgeMode(int newEdgeMode);
	void SetDecoSpace(int newDecoSpace);

	//Drawing Deco
	void ApplyDecoration(int x, int y, int colR, int colG, int colB, int colA, int mode);
	void ApplyDecorationPoint(int x, int y, int colR, int colG, int colB, int colA, int mode, Brush const &cBrush);
	void ApplyDecorationLine(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode, Brush const &cBrush);
	void ApplyDecorationBox(int x1, int y1, int x2, int y2, int colR, int colG, int colB, int colA, int mode);
	bool ColorCompare(Renderer *ren, int x, int y, int replaceR, int replaceG, int replaceB);
	void ApplyDecorationFill(Renderer *ren, int x, int y, int colR, int colG, int colB, int colA, int replaceR, int replaceG, int replaceB);

	//Drawing Tools like HEAT, AIR, and GRAV
	int Tool(int x, int y, int tool, int brushX, int brushY, float strength = 1.0f);
	int ToolBrush(int x, int y, int tool, Brush const &cBrush, float strength = 1.0f);
	void ToolLine(int x1, int y1, int x2, int y2, int tool, Brush const &cBrush, float strength = 1.0f);
	void ToolBox(int x1, int y1, int x2, int y2, int tool, float strength = 1.0f);

	//Drawing Walls
	int CreateWalls(int x, int y, int rx, int ry, int wall, Brush const *cBrush = nullptr);
	void CreateWallLine(int x1, int y1, int x2, int y2, int rx, int ry, int wall, Brush const *cBrush = nullptr);
	void CreateWallBox(int x1, int y1, int x2, int y2, int wall);
	int FloodWalls(int x, int y, int wall, int bm);

	//Drawing Particles
	int CreateParts(int positionX, int positionY, int c, Brush const &cBrush, int flags = -1);
	int CreateParts(int x, int y, int rx, int ry, int c, int flags = -1);
	int CreatePartFlags(int x, int y, int c, int flags);
	void CreateLine(int x1, int y1, int x2, int y2, int c, Brush const &cBrush, int flags = -1);
	void CreateLine(int x1, int y1, int x2, int y2, int c);
	void CreateBox(int x1, int y1, int x2, int y2, int c, int flags = -1);
	int FloodParts(int x, int y, int c, int cm, int flags = -1);


	void GetGravityField(int x, int y, float particleGrav, float newtonGrav, float & pGravX, float & pGravY);

	int GetParticleType(ByteString type);

	void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[]);
	void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[]);
	int get_wavelength_bin(int *wm);
	int get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny);
	int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny);
	void clear_sim();
	Simulation();
	~Simulation();

	bool InBounds(int x, int y);

	// These don't really belong anywhere at the moment, so go here for loop edge mode
	static int remainder_p(int x, int y);
	static float remainder_p(float x, float y);

	String ElementResolve(int type, int ctype) const;
	String BasicParticleInfo(Particle const &sample_part) const;


	struct CustomGOLData
	{
		int rule, colour1, colour2;
		String nameString, ruleString;

		inline bool operator <(const CustomGOLData &other) const
		{
			return rule < other.rule;
		}
	};

private:
	std::vector<CustomGOLData> customGol;

public:
	const CustomGOLData *GetCustomGOLByRule(int rule) const;
	const std::vector<CustomGOLData> GetCustomGol() { return customGol; }
	void SetCustomGOL(std::vector<CustomGOLData> newCustomGol);

private:
	CoordStack& getCoordStackSingleton();
};
