#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "Config.h"
#include "client/Client.h"
#include "simulation/Simulation.h"
#include "Graphics.h"
#include "interface/Point.h"

class Simulation;

class Graphics;

struct gcache_item
{
	int isready;
	int pixel_mode;
	int cola, colr, colg, colb;
	int firea, firer, fireg, fireb;
};
typedef struct gcache_item gcache_item;

class Renderer
{
public:
	std::vector<unsigned int> render_modes;
	unsigned int render_mode;
	unsigned int colour_mode;
	std::vector<unsigned int> display_modes;
	unsigned int display_mode;
	//
	unsigned char fire_r[YRES/CELL][XRES/CELL];
	unsigned char fire_g[YRES/CELL][XRES/CELL];
	unsigned char fire_b[YRES/CELL][XRES/CELL];
	unsigned int fire_alpha[CELL*3][CELL*3];
	char * flm_data;
	char * plasma_data;
	int emp_decor;
	//
	int decorations_enable;
	Simulation * sim;
	Graphics * g;
	gcache_item *graphicscache;

	//Zoom window
	ui::Point zoomWindowPosition;
	ui::Point zoomScopePosition;
	int zoomScopeSize;
	bool zoomEnabled;
	int ZFACTOR;

	//Renderers
	void RenderZoom();
	void DrawWalls();
	void DrawSigns();
	void render_gravlensing();
	void render_fire();
	void prepare_alpha(int size, float intensity);
	void render_parts();
	void draw_grav_zones();
	void draw_air();
	void draw_grav();
	void draw_other();

	//...
	void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);

	//Display mode modifiers
	void CompileDisplayMode();
	void CompileRenderMode();
	void AddRenderMode(unsigned int mode);
	void SetRenderMode(std::vector<unsigned int> render);
	void RemoveRenderMode(unsigned int mode);
	void AddDisplayMode(unsigned int mode);
	void RemoveDisplayMode(unsigned int mode);
	void SetDisplayMode(std::vector<unsigned int> display);
	void SetColourMode(unsigned int mode);

	Renderer(Graphics * g, Simulation * sim);
	~Renderer();
};

#endif
