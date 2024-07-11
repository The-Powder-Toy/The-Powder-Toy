#include <cmath>
#include "gui/game/RenderPreset.h"
#include "RasterDrawMethodsImpl.h"
#include "Renderer.h"
#include "simulation/ElementClasses.h"
#include "simulation/ElementGraphics.h"

void Renderer::RenderEnd()
{
	RenderZoom();
}

void Renderer::SetSample(Vec2<int> pos)
{
	sampleColor = GetPixel(pos);
}

void Renderer::clearScreen() {
	if(displayMode & DISPLAY_PERS)
	{
		std::copy(persistentVideo.begin(), persistentVideo.end(), video.RowIterator({ 0, 0 }));
	}
	else
	{
		std::fill_n(video.data(), WINDOWW * YRES, 0);
	}
}

void Renderer::RenderZoom()
{
	if(!zoomEnabled)
		return;
	{
		int x, y, i, j;
		pixel pix;

		DrawFilledRect(RectSized(zoomWindowPosition, { zoomScopeSize * ZFACTOR, zoomScopeSize * ZFACTOR }), 0x000000_rgb);
		DrawRect(RectSized(zoomWindowPosition - Vec2{ 2, 2 }, Vec2{ zoomScopeSize*ZFACTOR+3, zoomScopeSize*ZFACTOR+3 }), 0xC0C0C0_rgb);
		DrawRect(RectSized(zoomWindowPosition - Vec2{ 1, 1 }, Vec2{ zoomScopeSize*ZFACTOR+1, zoomScopeSize*ZFACTOR+1 }), 0x000000_rgb);
		for (j=0; j<zoomScopeSize; j++)
			for (i=0; i<zoomScopeSize; i++)
			{
				pix = video[{ i + zoomScopePosition.X, j + zoomScopePosition.Y }];
				for (y=0; y<ZFACTOR-1; y++)
					for (x=0; x<ZFACTOR-1; x++)
						video[{ i * ZFACTOR + x + zoomWindowPosition.X, j * ZFACTOR + y + zoomWindowPosition.Y }] = pix;
			}
		if (zoomEnabled)
		{
			for (j=-1; j<=zoomScopeSize; j++)
			{
				XorPixel(zoomScopePosition + Vec2{ j, -1 });
				XorPixel(zoomScopePosition + Vec2{ j, zoomScopeSize });
			}
			for (j=0; j<zoomScopeSize; j++)
			{
				XorPixel(zoomScopePosition + Vec2{ -1, j });
				XorPixel(zoomScopePosition + Vec2{ zoomScopeSize, j });
			}
		}
	}
}

void Renderer::DrawBlob(Vec2<int> pos, RGB<uint8_t> colour)
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

pixel Renderer::GetPixel(Vec2<int> pos) const
{
	if (pos.X<0 || pos.Y<0 || pos.X>=WINDOWW || pos.Y>=WINDOWH)
		return 0;
	return video[pos];
}

std::vector<RGB<uint8_t>> Renderer::flameTable;
std::vector<RGB<uint8_t>> Renderer::plasmaTable;
std::vector<RGB<uint8_t>> Renderer::heatTable;
std::vector<RGB<uint8_t>> Renderer::clfmTable;
std::vector<RGB<uint8_t>> Renderer::firwTable;
static bool tablesPopulated = false;
static std::mutex tablesPopulatedMx;
void Renderer::PopulateTables()
{
	std::lock_guard g(tablesPopulatedMx);
	if (!tablesPopulated)
	{
		tablesPopulated = true;
		flameTable = Graphics::Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x60300F_rgb, 0.50f },
			{ 0xDFBF6F_rgb, 0.90f },
			{ 0xAF9F0F_rgb, 1.00f },
		}, 200);
		plasmaTable = Graphics::Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x301040_rgb, 0.25f },
			{ 0x301060_rgb, 0.50f },
			{ 0xAFFFFF_rgb, 0.90f },
			{ 0xAFFFFF_rgb, 1.00f },
		}, 200);
		heatTable = Graphics::Gradient({
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
		clfmTable = Graphics::Gradient({
			{ 0x000000_rgb, 0.00f },
			{ 0x0A0917_rgb, 0.10f },
			{ 0x19163C_rgb, 0.20f },
			{ 0x28285E_rgb, 0.30f },
			{ 0x343E77_rgb, 0.40f },
			{ 0x49769A_rgb, 0.60f },
			{ 0x57A0B4_rgb, 0.80f },
			{ 0x5EC4C6_rgb, 1.00f },
		}, 200);
		firwTable = Graphics::Gradient({
			{ 0xFF00FF_rgb, 0.00f },
			{ 0x0000FF_rgb, 0.20f },
			{ 0x00FFFF_rgb, 0.40f },
			{ 0x00FF00_rgb, 0.60f },
			{ 0xFFFF00_rgb, 0.80f },
			{ 0xFF0000_rgb, 1.00f },
		}, 200);
	}
}

Renderer::Renderer():
	gravityZonesEnabled(false),
	gravityFieldEnabled(false),
	decorations_enable(1),
	blackDecorations(false),
	debugLines(false),
	sampleColor(0xFFFFFFFF),
    foundElements(0),
	mousePos(0, 0),
	zoomWindowPosition(0, 0),
	zoomScopePosition(0, 0),
	zoomScopeSize(32),
	zoomEnabled(false),
	ZFACTOR(8),
	gridSize(0)
{
	PopulateTables();

	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));

	//Set defauly display modes
	ResetModes();

	//Render mode presets. Possibly load from config in future?
	renderModePresets.push_back({
		"Alternative Velocity Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRC,
		0,
	});
	renderModePresets.push_back({
		"Velocity Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRV,
		0,
	});
	renderModePresets.push_back({
		"Pressure Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_AIRP,
		0,
	});
	renderModePresets.push_back({
		"Persistent Display",
		RENDER_EFFE | RENDER_BASC,
		DISPLAY_PERS,
		0,
	});
	renderModePresets.push_back({
		"Fire Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_EFFE | RENDER_BASC,
		0,
		0,
	});
	renderModePresets.push_back({
		"Blob Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_EFFE | RENDER_BLOB,
		0,
		0,
	});
	renderModePresets.push_back({
		"Heat Display",
		RENDER_BASC,
		DISPLAY_AIRH,
		COLOUR_HEAT,
	});
	renderModePresets.push_back({
		"Fancy Display",
		RENDER_FIRE | RENDER_SPRK | RENDER_GLOW | RENDER_BLUR | RENDER_EFFE | RENDER_BASC,
		DISPLAY_WARP,
		0,
	});
	renderModePresets.push_back({
		"Nothing Display",
		RENDER_BASC,
		0,
		0,
	});
	renderModePresets.push_back({
		"Heat Gradient Display",
		RENDER_BASC,
		0,
		COLOUR_GRAD,
	});
	renderModePresets.push_back({
		"Life Gradient Display",
		RENDER_BASC,
		0,
		COLOUR_LIFE,
	});

	prepare_alpha(CELL, 1.0f);
}

void Renderer::ClearAccumulation()
{
	std::fill(&fire_r[0][0], &fire_r[0][0] + NCELL, 0);
	std::fill(&fire_g[0][0], &fire_g[0][0] + NCELL, 0);
	std::fill(&fire_b[0][0], &fire_b[0][0] + NCELL, 0);
	std::fill(persistentVideo.begin(), persistentVideo.end(), 0);
}

void Renderer::SetRenderMode(uint32_t newRenderMode)
{
	int oldRenderMode = renderMode;
	renderMode = newRenderMode;
	if (!(renderMode & FIREMODE) && (oldRenderMode & FIREMODE))
	{
		ClearAccumulation();
	}
}

uint32_t Renderer::GetRenderMode()
{
	return renderMode;
}

void Renderer::SetDisplayMode(uint32_t newDisplayMode)
{
	int oldDisplayMode = displayMode;
	displayMode = newDisplayMode;
	if (!(displayMode & DISPLAY_PERS) && (oldDisplayMode & DISPLAY_PERS))
	{
		ClearAccumulation();
	}
}

uint32_t Renderer::GetDisplayMode()
{
	return displayMode;
}

void Renderer::SetColorMode(uint32_t newColorMode)
{
	colorMode = newColorMode;
}

uint32_t Renderer::GetColorMode()
{
	return colorMode;
}

void Renderer::ResetModes()
{
	SetRenderMode(RENDER_BASC | RENDER_FIRE | RENDER_SPRK | RENDER_EFFE);
	SetDisplayMode(0);
	SetColorMode(COLOUR_DEFAULT);
}

VideoBuffer Renderer::DumpFrame()
{
	VideoBuffer newBuffer(RES);
	newBuffer.BlendImage(video.data(), 0xFF, Size().OriginRect());
	return newBuffer;
}

template struct RasterDrawMethods<Renderer>;
