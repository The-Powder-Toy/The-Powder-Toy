#pragma once
#include <array>
#include <memory>
#include <mutex>
#include <vector>
#include "Graphics.h"
#include "gui/interface/Point.h"
#include "common/tpt-rand.h"
#include "SimulationConfig.h"

class RenderPreset;
class Simulation;

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

int HeatToColour(float temp);

class Renderer: public RasterDrawMethods<Renderer>
{
	PlaneAdapter<std::array<pixel, WINDOW.X * RES.Y>, WINDOW.X, RES.Y> video;
	std::array<pixel, WINDOW.X * RES.Y> persistentVideo;
	PlaneAdapter<std::array<pixel, WINDOW.X * RES.Y>, WINDOW.X, RES.Y> warpVideo;

	Rect<int> getClipRect() const
	{
		return video.Size().OriginRect();
	}

	friend struct RasterDrawMethods<Renderer>;

public:
	Vec2<int> Size() const
	{
		return video.Size();
	}

	pixel const *Data() const
	{
		return video.data();
	}

	RNG rng;

	Simulation * sim;
	gcache_item *graphicscache;

	std::vector<unsigned int> render_modes;
	unsigned int render_mode;
	unsigned int colour_mode;
	std::vector<unsigned int> display_modes;
	unsigned int display_mode;
	std::vector<RenderPreset> renderModePresets;
	//
	unsigned char fire_r[YCELLS][XCELLS];
	unsigned char fire_g[YCELLS][XCELLS];
	unsigned char fire_b[YCELLS][XCELLS];
	unsigned int fire_alpha[CELL*3][CELL*3];
	//
	bool gravityZonesEnabled;
	bool gravityFieldEnabled;
	int decorations_enable;
	bool blackDecorations;
	bool debugLines;
	pixel sampleColor;
	int findingElement;
	int foundElements;

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
	void clearScreen();
	void SetSample(int x, int y);

	[[deprecated("Use video")]]
	pixel *const vid = video.Base.data();
	[[deprecated("Use persistentVideo")]]
	pixel *const persistentVid = persistentVideo.data();
	[[deprecated("Use wrapVideo")]]
	pixel *const warpVid = warpVideo.data();

	void draw_icon(int x, int y, Icon icon);

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

	void ResetModes();

	int GetGridSize() { return gridSize; }
	void SetGridSize(int value) { gridSize = value; }

	static std::unique_ptr<VideoBuffer> WallIcon(int wallID, Vec2<int> size);

	Renderer(Simulation * sim);
	~Renderer();

#define RENDERER_TABLE(name) \
	static std::vector<RGB<uint8_t>> name; \
	static inline RGB<uint8_t> name ## At(int index) \
	{ \
		auto size = int(name.size()); \
		if (index <        0) index =        0; \
		if (index > size - 1) index = size - 1; \
		return name[index]; \
	}
	RENDERER_TABLE(flameTable)
	RENDERER_TABLE(plasmaTable)
	RENDERER_TABLE(heatTable)
	RENDERER_TABLE(clfmTable)
	RENDERER_TABLE(firwTable)
#undef RENDERER_TABLE
	static void PopulateTables();

private:
	int gridSize;
};
