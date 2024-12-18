#pragma once
#include <array>
#include <memory>
#include <span>
#include <vector>
#include "common/Plane.h"
#include "common/String.h"
#include "gui/interface/Point.h"
#include "Icons.h"
#include "Pixel.h"
#include "RasterDrawMethods.h"
#include "SimulationConfig.h"

class VideoBuffer: public RasterDrawMethods<VideoBuffer>
{
	PlaneAdapter<std::vector<pixel>> video;

	Rect<int> GetClipRect() const
	{
		return video.Size().OriginRect();
	}

	friend struct RasterDrawMethods<VideoBuffer>;

public:
	VideoBuffer(pixel const *data, Vec2<int> size);
	VideoBuffer(pixel const *data, Vec2<int> size, size_t rowStride);
	VideoBuffer(Vec2<int> size);

	template<class Plane>
	VideoBuffer(const Plane &plane) : VideoBuffer(plane.data(), plane.Size())
	{
	}

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

	static std::unique_ptr<VideoBuffer> FromPNG(std::span<const char> data);
	std::unique_ptr<std::vector<char>> ToPNG() const;
	std::vector<char> ToPPM() const;
};
