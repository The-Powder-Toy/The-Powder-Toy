#pragma once
#include <array>
#include <memory>
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

public:
	VideoBuffer(VideoBuffer const &) = default;
	VideoBuffer(pixel const *data, Vec2<int> size);
	VideoBuffer(pixel const *data, Vec2<int> size, size_t rowStride);
	VideoBuffer(Vec2<int> size);

	Vec2<int> Size() const
	{
		return video.Size();
	}

	pixel *Data()
	{
		return video.data();
	}

	pixel const *Data() const
	{
		return video.data();
	}

	void Crop(Rect<int>);

	void Resize(float factor, bool resample = false);
	void Resize(Vec2<int> size, bool resample = false);
	// Automatically choose a size to fit within the given box, keeping aspect ratio
	void ResizeToFit(Vec2<int> bound, bool resample = false);

	static std::unique_ptr<VideoBuffer> FromPNG(std::vector<char> const &);
	std::unique_ptr<std::vector<char>> ToPNG() const;
	std::vector<char> ToPPM() const;
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

public:
	Vec2<int> Size() const
	{
		return video.Size();
	}

	pixel const *Data() const
	{
		return video.data();
	}

	pixel *Data()
	{
		return video.data();
	}

	struct GradientStop
	{
		RGB<uint8_t> color;
		float point;

		bool operator <(const GradientStop &other) const;
	};
	static std::vector<RGB<uint8_t>> Gradient(std::vector<GradientStop> stops, int resolution);

	//Font/text metrics
	[[deprecated("Use TextSize().X")]]
	static int textwidth(const String &s);

	VideoBuffer DumpFrame();

	void draw_icon(int x, int y, Icon icon, unsigned char alpha = 255, bool invert = false);

	void Finalise();

	Graphics();

	void SwapClipRect(Rect<int> &);
};
