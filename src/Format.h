#pragma once
#include <memory>
#include <vector>
#include "common/String.h"
#include "common/Plane.h"
#include "graphics/Pixel.h"

class VideoBuffer;

namespace format
{
	ByteString URLEncode(ByteString value);
	ByteString URLDecode(ByteString value);
	ByteString UnixtimeToDate(time_t unixtime, ByteString dateFomat = ByteString("%d %b %Y"));
	ByteString UnixtimeToDateMini(time_t unixtime);
	String CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric = false);
	std::vector<char> PixelsToPPM(PlaneAdapter<std::vector<pixel>> const &);
	std::unique_ptr<std::vector<char>> PixelsToPNG(PlaneAdapter<std::vector<pixel>> const &);
	std::unique_ptr<PlaneAdapter<std::vector<pixel_rgba>>> PixelsFromPNG(std::vector<char> const &);
	std::unique_ptr<PlaneAdapter<std::vector<pixel>>> PixelsFromPNG(std::vector<char> const &, RGB<uint8_t> background);
	void RenderTemperature(StringBuilder &sb, float temp, int scale);
	float StringToTemperature(String str, int defaultScale);
}
