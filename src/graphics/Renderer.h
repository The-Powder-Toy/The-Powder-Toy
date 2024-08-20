#pragma once
#include "Icons.h"
#include "RasterDrawMethods.h"
#include "gui/game/RenderPreset.h"
#include "gui/interface/Point.h"
#include "common/tpt-rand.h"
#include "RendererFrame.h"
#include "FindingElement.h"
#include <cstdint>
#include <optional>
#include <memory>
#include <mutex>
#include <vector>

struct RenderPreset;
struct RenderableSimulation;
class Renderer;
struct Particle;

struct GraphicsFuncContext
{
	const Renderer *ren;
	const RenderableSimulation *sim;
	RNG rng;
	const Particle *pipeSubcallCpart;
	Particle *pipeSubcallTpart;
};

int HeatToColour(float temp);

class Renderer: public RasterDrawMethods<Renderer>
{
	RendererFrame video;
	std::array<pixel, WINDOW.X * RES.Y> persistentVideo;
	RendererFrame warpVideo;

	Rect<int> GetClipRect() const
	{
		return video.Size().OriginRect();
	}

	friend struct RasterDrawMethods<Renderer>;

	float fireIntensity = 1;

public:
	RNG rng;
	const RenderableSimulation *sim = nullptr;

	const RendererFrame &GetVideo() const
	{
		return video;
	}

	uint32_t renderMode = 0;
	uint32_t colorMode = 0;
	uint32_t displayMode = 0;
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
	std::optional<FindingElement> findingElement;
	int foundElements;

	//Mouse position for debug information
	ui::Point mousePos;

	//Renderers
	void RenderSimulation();

	void DrawBlob(Vec2<int> pos, RGB<uint8_t> colour);
	void DrawWalls();
	void DrawSigns();
	void render_gravlensing(const RendererFrame &source);
	void render_fire();
	float GetFireIntensity() const
	{
		return fireIntensity;
	}
	void prepare_alpha(int size, float intensity);
	void render_parts();
	void draw_grav_zones();
	void draw_air();
	void draw_grav();
	void draw_other();

	void ClearAccumulation();
	void clearScreen();

	void draw_icon(int x, int y, Icon icon);

	//...
	//Display mode modifiers
	void SetRenderMode(uint32_t newRenderMode);
	uint32_t GetRenderMode();
	void SetDisplayMode(uint32_t newDisplayMode);
	uint32_t GetDisplayMode();
	void SetColorMode(uint32_t newColorMode);
	uint32_t GetColorMode();

	void ResetModes();

	int GetGridSize() { return gridSize; }
	void SetGridSize(int value) { gridSize = value; }

	static std::unique_ptr<VideoBuffer> WallIcon(int wallID, Vec2<int> size);

	Renderer();

	struct GradientStop
	{
		RGB<uint8_t> color;
		float point;

		bool operator <(const GradientStop &other) const;
	};
	static std::vector<RGB<uint8_t>> Gradient(std::vector<GradientStop> stops, int resolution);

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
