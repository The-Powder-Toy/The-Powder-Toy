#pragma once
#include <array>
#include <vector>
#include "common/Plane.h"
#include "common/String.h"
#include "common/tpt-inline.h"
#include "Icons.h"
#include "Pixel.h"
#include "RasterDrawMethods.h"
#include "SimulationConfig.h"

class VideoBuffer: public RasterDrawMethods<VideoBuffer>
{
	PlaneAdapter<std::vector<pixel>> video;

	Rect<int> getClipRect() const
	{
		return video.Size().OriginRect();
	}

	friend struct RasterDrawMethods<VideoBuffer>;

	void CopyData(pixel * buffer, int width, int height, int pitch);

public:
	[[deprecated("Use video")]]
	std::vector<pixel> &Buffer = video.Base;
	[[deprecated("Use Size()")]]
	size_t &Width = video.xExtent<DynamicExtent>::extent; // See TODO in common/Plane.h
	[[deprecated("Use Size()")]]
	size_t &Height = video.yExtent<DynamicExtent>::extent;

	VideoBuffer(VideoBuffer const &) = default;
	VideoBuffer(pixel const *data, Vec2<int> size);
	VideoBuffer(pixel const *data, Vec2<int> size, size_t rowStride);
	VideoBuffer(Vec2<int> size);

	Vec2<int> Size() const
	{
		return video.Size();
	}

	pixel const *Data() const
	{
		return video.data();
	}

	void Crop(Rect<int>);

	void Resize(float factor, bool resample = false);
	void Resize(Vec2<int> size, bool resamble = false, bool fixedRatio = true);

	[[deprecated("Use VideoBuffer(VideoBuffer const &)")]]
	VideoBuffer(VideoBuffer * old);
	[[deprecated("Use VideoBuffer(pixel const *, Vec2<int>)")]]
	VideoBuffer(pixel const *buffer, int width, int height, int pitch = 0);
	[[deprecated("Use VideoBuffer(Vec2<int>)")]]
	VideoBuffer(int width, int height);
	[[deprecated("Use Resize(Vec2<int>, bool, bool)")]]
	void Resize(int width, int height, bool resample = false, bool fixedRatio = true);
	[[deprecated("Use Crop(Rect<int>)")]]
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

	bool WritePNG(const ByteString &path) const;
};

class Graphics: public RasterDrawMethods<Graphics>
{
	PlaneAdapter<std::array<pixel, WINDOW.X * WINDOW.Y>, WINDOW.X, WINDOW.Y> video;
	Rect<int> clipRect = video.Size().OriginRect();

	Rect<int> getClipRect() const
	{
		return clipRect;
	}

	friend struct RasterDrawMethods<Graphics>;

	[[deprecated("Use clipRect")]]
	int &clipx1 = clipRect.TopLeft.X;
	[[deprecated("Use clipRect")]]
	int &clipy1 = clipRect.TopLeft.Y;
	[[deprecated("Use clipRect")]]
	int &clipx2 = clipRect.BottomRight.X;
	[[deprecated("Use clipRect")]]
	int &clipy2 = clipRect.BottomRight.Y;

public:
	pixel const *Data() const
	{
		return video.data();
	}

	[[deprecated("Use Data()")]]
	pixel *vid = video.data();

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

	Graphics()
	{}

	void SetClipRect(int &x, int &y, int &w, int &h);
};

bool PngDataToPixels(std::vector<pixel> &imageData, int &imgw, int &imgh, const char *pngData, size_t pngDataSize, bool addBackground);
