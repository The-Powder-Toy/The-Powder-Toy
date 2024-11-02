#include <cmath>
#include "gui/game/RenderPreset.h"
#include "RasterDrawMethodsImpl.h"
#include "Renderer.h"
#include "simulation/ElementClasses.h"
#include "simulation/ElementGraphics.h"

const std::vector<RenderPreset> Renderer::renderModePresets = {
	{
		"Alternative Velocity Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRC,
		0,
	},
	{
		"Velocity Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRV,
		0,
	},
	{
		"Pressure Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRP,
		0,
	},
	{
		"Persistent Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_PERS,
		0,
	},
	{
		"Fire Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_EFFE | RENDER_BASC,
		0,
		0,
	},
	{
		"Blob Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_EFFE | RENDER_BLOB,
		0,
		0,
	},
	{
		"Heat Display",
		RENDER_BASC,
		DISPLAY_AIRH,
		COLOUR_HEAT,
	},
	{
		"Fancy Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_GLOW | RENDER_BLUR | RENDER_EFFE | RENDER_BASC,
		DISPLAY_WARP,
		0,
	},
	{
		"Nothing Display",
		RENDER_BASC,
		0,
		0,
	},
	{
		"Heat Gradient Display",
		RENDER_BASC,
		0,
		COLOUR_GRAD,
	},
	{
		"Life Gradient Display",
		RENDER_BASC,
		0,
		COLOUR_LIFE,
	},
};

void Renderer::Clear()
{
	if(displayMode & DISPLAY_PERS)
	{
		std::copy(persistentVideo.begin(), persistentVideo.end(), video.RowIterator({ 0, 0 }));
	}
	else
	{
		std::fill_n(video.data(), WINDOWW * YRES, 0);
	}
}

void Renderer::DrawBlob(Vec2<int> pos, RGB colour)
{
	BlendPixel(pos + Vec2{ +1,  0 }, colour.WithAlpha(112));
	BlendPixel(pos + Vec2{ -1,  0 }, colour.WithAlpha(112));
	BlendPixel(pos + Vec2{  0,  1 }, colour.WithAlpha(112));
	BlendPixel(pos + Vec2{  0, -1 }, colour.WithAlpha(112));
	BlendPixel(pos + Vec2{  1, -1 }, colour.WithAlpha(64));
	BlendPixel(pos + Vec2{ -1, -1 }, colour.WithAlpha(64));
	BlendPixel(pos + Vec2{  1,  1 }, colour.WithAlpha(64));
	BlendPixel(pos + Vec2{ -1, +1 }, colour.WithAlpha(64));
}

float temp[CELL*3][CELL*3];
float fire_alphaf[CELL*3][CELL*3];
float glow_alphaf[11][11];
float blur_alphaf[7][7];
void Renderer::prepare_alpha(int size, float intensity)
{
	fireIntensity = intensity;
	//TODO: implement size
	int x,y,i,j;
	float multiplier = 255.0f*fireIntensity;

	memset(temp, 0, sizeof(temp));
	for (x=0; x<CELL; x++)
		for (y=0; y<CELL; y++)
			for (i=-CELL; i<CELL; i++)
				for (j=-CELL; j<CELL; j++)
					temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
	for (x=0; x<CELL*3; x++)
		for (y=0; y<CELL*3; y++)
			fire_alpha[y][x] = (int)(multiplier*temp[y][x]/(CELL*CELL));

}

std::vector<RGB> Renderer::flameTable;
std::vector<RGB> Renderer::plasmaTable;
std::vector<RGB> Renderer::heatTable;
std::vector<RGB> Renderer::clfmTable;
std::vector<RGB> Renderer::firwTable;
static bool tablesPopulated = false;
static std::mutex tablesPopulatedMx;
void Renderer::PopulateTables()
{
	std::lock_guard g(tablesPopulatedMx);
	if (!tablesPopulated)
	{
		tablesPopulated = true;
		flameTable = Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x60300F_rgb, 0.50f },
			{ 0xDFBF6F_rgb, 0.90f },
			{ 0xAF9F0F_rgb, 1.00f },
		}, 200);
		plasmaTable = Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x301040_rgb, 0.25f },
			{ 0x301060_rgb, 0.50f },
			{ 0xAFFFFF_rgb, 0.90f },
			{ 0xAFFFFF_rgb, 1.00f },
		}, 200);
		heatTable = Gradient({
			{ 0x2B00FF_rgb, 0.00f },
			{ 0x003CFF_rgb, 0.01f },
			{ 0x00C0FF_rgb, 0.05f },
			{ 0x00FFEB_rgb, 0.08f },
			{ 0x00FF14_rgb, 0.19f },
			{ 0x4BFF00_rgb, 0.25f },
			{ 0xC8FF00_rgb, 0.37f },
			{ 0xFFDC00_rgb, 0.45f },
			{ 0xFF0000_rgb, 0.71f },
			{ 0xFF00DC_rgb, 1.00f },
		}, 1024);
		clfmTable = Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x0A0917_rgb, 0.10f },
			{ 0x19163C_rgb, 0.20f },
			{ 0x28285E_rgb, 0.30f },
			{ 0x343E77_rgb, 0.40f },
			{ 0x49769A_rgb, 0.60f },
			{ 0x57A0B4_rgb, 0.80f },
			{ 0x5EC4C6_rgb, 1.00f },
		}, 200);
		firwTable = Gradient({
			{ 0xFF00FF_rgb, 0.00f },
			{ 0x0000FF_rgb, 0.20f },
			{ 0x00FFFF_rgb, 0.40f },
			{ 0x00FF00_rgb, 0.60f },
			{ 0xFFFF00_rgb, 0.80f },
			{ 0xFF0000_rgb, 1.00f },
		}, 200);
	}
}

Renderer::Renderer()
{
	PopulateTables();

	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));

	//Set defauly display modes
	prepare_alpha(CELL, 1.0f);
	ClearAccumulation();
}

void Renderer::ClearAccumulation()
{
	std::fill(&fire_r[0][0], &fire_r[0][0] + NCELL, 0);
	std::fill(&fire_g[0][0], &fire_g[0][0] + NCELL, 0);
	std::fill(&fire_b[0][0], &fire_b[0][0] + NCELL, 0);
	std::fill(persistentVideo.begin(), persistentVideo.end(), 0);
}

void Renderer::ApplySettings(const RendererSettings &newSettings)
{
	if (!(newSettings.renderMode & FIREMODE) && (renderMode & FIREMODE))
	{
		ClearAccumulation();
	}
	if (!(newSettings.displayMode & DISPLAY_PERS) && (displayMode & DISPLAY_PERS))
	{
		ClearAccumulation();
	}
	static_cast<RendererSettings &>(*this) = newSettings;
}

template struct RasterDrawMethods<Renderer>;

bool Renderer::GradientStop::operator <(const GradientStop &other) const
{
	return point < other.point;
}

std::vector<RGB> Renderer::Gradient(std::vector<GradientStop> stops, int resolution)
{
	std::vector<RGB> table(resolution, 0x000000_rgb);
	if (stops.size() >= 2)
	{
		std::sort(stops.begin(), stops.end());
		auto stop = -1;
		for (auto i = 0; i < resolution; ++i)
		{
			auto point = i / (float)resolution;
			while (stop < (int)stops.size() - 1 && stops[stop + 1].point <= point)
			{
				++stop;
			}
			if (stop < 0 || stop >= (int)stops.size() - 1)
			{
				continue;
			}
			auto &left = stops[stop];
			auto &right = stops[stop + 1];
			auto f = (point - left.point) / (right.point - left.point);
			table[i] = left.color.Blend(right.color.WithAlpha(uint8_t(f * 0xFF)));
		}
	}
	return table;
}
