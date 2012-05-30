#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#if defined(OGLR)
#include "OpenGLHeaders.h"
#endif

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
	void FinaliseParts();
	void clearScreen(float alpha);

	//class SolidsRenderer;

#ifdef OGLR
	void checkShader(GLuint shader, char * shname);
	void checkProgram(GLuint program, char * progname);
	void loadShaders();
#endif

	void drawblob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);
	//...
	void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);

	//Display mode modifiers
	void CompileDisplayMode();
	void CompileRenderMode();
	void AddRenderMode(unsigned int mode);
	void SetRenderMode(std::vector<unsigned int> render);
	std::vector<unsigned int> GetRenderMode();
	void RemoveRenderMode(unsigned int mode);
	void AddDisplayMode(unsigned int mode);
	void RemoveDisplayMode(unsigned int mode);
	void SetDisplayMode(std::vector<unsigned int> display);
	std::vector<unsigned int> GetDisplayMode();
	void SetColourMode(unsigned int mode);
	unsigned int GetColourMode();

	Renderer(Graphics * g, Simulation * sim);
	~Renderer();

private:
#ifdef OGLR
	GLuint zoomTex, airBuf, fireAlpha, glowAlpha, blurAlpha, partsFboTex, partsFbo, partsTFX, partsTFY, airPV, airVY, airVX;
	GLuint fireProg, airProg_Pressure, airProg_Velocity, airProg_Cracker, lensProg;
	GLuint fireV[(YRES*XRES)*2];
	GLfloat fireC[(YRES*XRES)*4];
	GLuint smokeV[(YRES*XRES)*2];
	GLfloat smokeC[(YRES*XRES)*4];
	GLuint blobV[(YRES*XRES)*2];
	GLfloat blobC[(YRES*XRES)*4];
	GLuint blurV[(YRES*XRES)*2];
	GLfloat blurC[(YRES*XRES)*4];
	GLuint glowV[(YRES*XRES)*2];
	GLfloat glowC[(YRES*XRES)*4];
	GLuint flatV[(YRES*XRES)*2];
	GLfloat flatC[(YRES*XRES)*4];
	GLuint addV[(YRES*XRES)*2];
	GLfloat addC[(YRES*XRES)*4];
	GLfloat lineV[(((YRES*XRES)*2)*6)];
	GLfloat lineC[(((YRES*XRES)*2)*6)];
#endif
};

#endif
