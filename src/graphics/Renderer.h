#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#ifdef OGLR
#include "OpenGLHeaders.h"
#endif

#include "Config.h"
#include "Graphics.h"
#include "gui/interface/Point.h"

class RenderPreset;
class Simulation;
class Graphics;

struct gcache_item
{
	int isready;
	int pixel_mode;
	int cola, colr, colg, colb;
	int firea, firer, fireg, fireb;
	gcache_item() :
	isready(0),
	pixel_mode(0),
	cola(0),
	colr(0),
	colg(0),
	colb(0),
	firea(0),
	firer(0),
	fireg(0),
	fireb(0)
	{
	}
};
typedef struct gcache_item gcache_item;

class Renderer
{
public:
	Simulation * sim;
	Graphics * g;
	gcache_item *graphicscache;

	std::vector<unsigned int> render_modes;
	unsigned int render_mode;
	unsigned int colour_mode;
	std::vector<unsigned int> display_modes;
	unsigned int display_mode;
	RenderPreset * renderModePresets;
	//
	unsigned char fire_r[YRES/CELL][XRES/CELL];
	unsigned char fire_g[YRES/CELL][XRES/CELL];
	unsigned char fire_b[YRES/CELL][XRES/CELL];
	unsigned int fire_alpha[CELL*3][CELL*3];
	char * flm_data;
	char * plasma_data;
	//
	bool gravityZonesEnabled;
	bool gravityFieldEnabled;
	int decorations_enable;
	bool blackDecorations;
	bool debugLines;
	pixel sampleColor;
	int findingElement;

	//Mouse position for debug information
	ui::Point mousePos;

	//Zoom window
	ui::Point zoomWindowPosition;
	ui::Point zoomScopePosition;
	int zoomScopeSize;
	bool zoomEnabled;
	int ZFACTOR;

	//Renderers
	void RenderBegin();
	void RenderEnd();

	void RenderZoom();
	void DrawBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);
	void DrawWalls();
	void DrawSigns();
	void render_gravlensing(pixel * source);
	void render_fire();
	void prepare_alpha(int size, float intensity);
	void render_parts();
	void draw_grav_zones();
	void draw_air();
	void draw_grav();
	void draw_other();
	void FinaliseParts();

	void ClearAccumulation();
	void clearScreen(float alpha);
	void SetSample(int x, int y);

#ifdef OGLR
	void checkShader(GLuint shader, const char * shname);
	void checkProgram(GLuint program, const char * progname);
	void loadShaders();
	GLuint vidBuf,textTexture;
	GLint prevFbo;
#endif
	pixel * vid;
	pixel * persistentVid;
	pixel * warpVid;
	void blendpixel(int x, int y, int r, int g, int b, int a);
	void addpixel(int x, int y, int r, int g, int b, int a);

	void draw_icon(int x, int y, Icon icon);

	int drawtext_outline(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext(int x, int y, std::string s, int r, int g, int b, int a);
	int drawchar(int x, int y, int c, int r, int g, int b, int a);
	int addchar(int x, int y, int c, int r, int g, int b, int a);

	void xor_pixel(int x, int y);
	void xor_line(int x, int y, int x2, int y2);
	void xor_rect(int x, int y, int width, int height);
	void xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h);

	void draw_line(int x, int y, int x2, int y2, int r, int g, int b, int a);
	void drawrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void fillrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void drawcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void fillcircle(int x, int y, int rx, int ry, int r, int g, int b, int a);
	void clearrect(int x, int y, int width, int height);
	void gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2);

	void draw_image(pixel *img, int x, int y, int w, int h, int a);
	void draw_image(const VideoBuffer & vidBuf, int w, int h, int a);
	void draw_image(VideoBuffer * vidBuf, int w, int h, int a);

	VideoBuffer DumpFrame();

	void drawblob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb);

	pixel GetPixel(int x, int y);
	//...
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

	int GetGridSize() { return gridSize; }
	void SetGridSize(int value) { gridSize = value; }

	static VideoBuffer * WallIcon(int wallID, int width, int height);

	Renderer(Graphics * g, Simulation * sim);
	~Renderer();

private:
	int gridSize;
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
