#pragma once
#include <memory>
#include <span>
#include <vector>
#include "common/String.h"
#include "common/Plane.h"
#include "graphics/Pixel.h"
#include "simulation/SimulationSettings.h"

class VideoBuffer;

namespace format
{
	ByteString URLEncode(ByteString value);
	ByteString URLDecode(ByteString value);
	ByteString UnixtimeToDate(time_t unixtime, ByteString dateFomat = ByteString("%d %b %Y"), bool local = true);
	ByteString UnixtimeToDateMini(time_t unixtime);
	String CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric = false);
	std::vector<char> PixelsToPPM(PlaneAdapter<std::vector<pixel>> const &);
	std::unique_ptr<std::vector<char>> PixelsToPNG(PlaneAdapter<std::vector<pixel>> const &);
	std::unique_ptr<PlaneAdapter<std::vector<pixel_rgba>>> PixelsFromPNG(std::span<const char> data);
	std::unique_ptr<PlaneAdapter<std::vector<pixel>>> PixelsFromPNG(std::span<const char> data, RGB background);
	void RenderTemperature(StringBuilder &sb, float temp, TempScale scale);
	float StringToTemperature(String str, TempScale defaultScale);
}
