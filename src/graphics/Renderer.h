#pragma once
#include "Icons.h"
#include "RasterDrawMethods.h"
#include "gui/game/RenderPreset.h"
#include "RendererSettings.h"
#include "common/tpt-rand.h"
#include "RendererFrame.h"
#include <cstdint>
#include <optional>
#include <memory>
#include <mutex>
#include <vector>

struct RenderPreset;
class Renderer;
struct RenderableSimulation;
struct Particle;

struct GraphicsFuncContext
{
	const RendererSettings *ren;
	const RenderableSimulation *sim;
	RNG rng;
	const Particle *pipeSubcallCpart;
	Particle *pipeSubcallTpart;
};

int HeatToColour(float temp);

class Renderer : private RendererSettings, public RasterDrawMethods<Renderer>
{
	RendererFrame video;
	std::array<pixel, WINDOW.X * RES.Y> persistentVideo;
	RendererFrame warpVideo;
	int foundParticles = 0;

	Rect<int> GetClipRect() const
	{
		return video.Size().OriginRect();
	}

	friend struct RasterDrawMethods<Renderer>;

	RNG rng;
	unsigned char fire_r[YCELLS][XCELLS];
	unsigned char fire_g[YCELLS][XCELLS];
	unsigned char fire_b[YCELLS][XCELLS];
	unsigned int fire_alpha[CELL*3][CELL*3];

	void DrawBlob(Vec2<int> pos, RGB colour);
	void DrawWalls();
	void DrawSigns();
	void render_gravlensing(const RendererFrame &source);
	void render_fire();
	void prepare_alpha(int size, float intensity);
	void render_parts();
	void draw_grav_zones();
	void draw_air();
	void draw_grav();
	void draw_other();

public:
	Renderer();
	void ApplySettings(const RendererSettings &newSettings);
	void RenderSimulation();
	void RenderBackground();
	void ApproximateAccumulation();
	void ClearAccumulation();
	void Clear();

	const RendererFrame &GetVideo() const
	{
		return video;
	}

	int GetFoundParticles() const
	{
		return foundParticles;
	}

	const RenderableSimulation *sim = nullptr;

	struct GradientStop
	{
		RGB color;
		float point;

		bool operator <(const GradientStop &other) const;
	};
	static std::vector<RGB> Gradient(std::vector<GradientStop> stops, int resolution);
	static std::unique_ptr<VideoBuffer> WallIcon(int wallID, Vec2<int> size);
	static const std::vector<RenderPreset> renderModePresets;

#define RENDERER_TABLE(name) \
	static std::vector<RGB> name; \
	static inline RGB name ## At(int index) \
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
};
