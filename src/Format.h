#pragma once
#include "Config.h"

#include "common/String.h"
#include <vector>

class VideoBuffer;

namespace format
{
	ByteString URLEncode(ByteString value);
	ByteString URLDecode(ByteString value);
	ByteString UnixtimeToDate(time_t unixtime, ByteString dateFomat = ByteString("%d %b %Y"));
	ByteString UnixtimeToDateMini(time_t unixtime);
	String CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric = false);
	std::vector<char> VideoBufferToPPM(const VideoBuffer & vidBuf);
	void RenderTemperature(StringBuilder &sb, float temp, int scale);
	float StringToTemperature(String str, int defaultScale);
}
