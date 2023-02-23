#pragma once
#include "common/Geometry.h"
#include "common/String.h"
#include "common/tpt-inline.h"
#include "Icons.h"
#include "Pixel.h"
#include "RasterDrawMethods.h"
#include "SimulationConfig.h"

//"Graphics lite" - slightly lower performance due to variable size,
class VideoBuffer
{
public:
	pixel * Buffer;
	int Width, Height;

	VideoBuffer(const VideoBuffer & old);
	VideoBuffer(VideoBuffer * old);
	VideoBuffer(pixel * buffer, int width, int height, int pitch = 0);
	VideoBuffer(int width, int height);
	void Resize(float factor, bool resample = false);
	void Resize(int width, int height, bool resample = false, bool fixedRatio = true);
	void Crop(int width, int height, int x, int y);
	TPT_INLINE void BlendPixel(int x, int y, int r, int g, int b, int a)
	{
		pixel t;
		if (x<0 || y<0 || x>=Width || y>=Height)
			return;
		if (a!=255)
		{
			t = Buffer[y*(Width)+x];
			r = (a*r + (255-a)*PIXR(t)) >> 8;
			g = (a*g + (255-a)*PIXG(t)) >> 8;
			b = (a*b + (255-a)*PIXB(t)) >> 8;
		}
		Buffer[y*(Width)+x] = PIXRGB(r,g,b);
	}

	TPT_INLINE void SetPixel(int x, int y, int r, int g, int b, int a)
	{
		if (x<0 || y<0 || x>=Width || y>=Height)
				return;
		Buffer[y*(Width)+x] = PIXRGB((r*a)>>8, (g*a)>>8, (b*a)>>8);
	}

	TPT_INLINE void AddPixel(int x, int y, int r, int g, int b, int a)
	{
		pixel t;
		if (x<0 || y<0 || x>=Width || y>=Height)
			return;
		t = Buffer[y*(Width)+x];
		r = (a*r + 255*PIXR(t)) >> 8;
		g = (a*g + 255*PIXG(t)) >> 8;
		b = (a*b + 255*PIXB(t)) >> 8;
		if (r>255)
			r = 255;
		if (g>255)
			g = 255;
		if (b>255)
			b = 255;
		Buffer[y*(Width)+x] = PIXRGB(r,g,b);
	}
	int SetCharacter(int x, int y, String::value_type c, int r, int g, int b, int a);
	int BlendCharacter(int x, int y, String::value_type c, int r, int g, int b, int a);
	int AddCharacter(int x, int y, String::value_type c, int r, int g, int b, int a);
	~VideoBuffer();

	void CopyData(pixel * buffer, int width, int height, int pitch);
	bool WritePNG(const ByteString &path) const;
};

class Graphics: public RasterDrawMethods<Graphics>
{
public:
	constexpr static auto VIDXRES = WINDOW.X;

	Rect<int> clip;

	pixel *vid;
	int sdl_scale;

	struct GradientStop
	{
		pixel color;
		float point;

		bool operator <(const GradientStop &other) const;
	};
	static std::vector<pixel> Gradient(std::vector<GradientStop> stops, int resolution);

	//PTIF methods
	static pixel *resample_img_nn(pixel *src, int sw, int sh, int rw, int rh);
	static pixel *resample_img(pixel *src, int sw, int sh, int rw, int rh);

	//Font/text metrics
	static int CharWidth(String::value_type c);
	static int textwidthx(const String &s, int w);
	static int textwidth(const String &s);
	static void textsize(const String &s, int & width, int & height);

	VideoBuffer DumpFrame();

	void draw_icon(int x, int y, Icon icon, unsigned char alpha = 255, bool invert = false);

	void Clear();
	void Finalise();

	void draw_rgba_image(const pixel *data, int w, int h, int x, int y, float alpha);

	Graphics();
	~Graphics();

	void SetClipRect(Rect<int> &);
};

bool PngDataToPixels(std::vector<pixel> &imageData, int &imgw, int &imgh, const char *pngData, size_t pngDataSize, bool addBackground);
