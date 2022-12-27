#include "Format.h"

#include <ctime>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <cstring>
#include <zlib.h>
#include <cstdio>

#include "graphics/Graphics.h"

#ifndef RENDERER
# include "SDLCompat.h"
#endif

ByteString format::UnixtimeToDate(time_t unixtime, ByteString dateFormat)
{
	struct tm * timeData;
	char buffer[128];

	timeData = localtime(&unixtime);

	strftime(buffer, 128, dateFormat.c_str(), timeData);
	return ByteString(buffer);
}

ByteString format::UnixtimeToDateMini(time_t unixtime)
{
	time_t currentTime = time(NULL);
	struct tm currentTimeData = *gmtime(&currentTime);
	struct tm timeData = *gmtime(&unixtime);

	if(currentTimeData.tm_year != timeData.tm_year)
	{
		return UnixtimeToDate(unixtime, "%d %b %Y");
	}
	else if(currentTimeData.tm_mon != timeData.tm_mon || currentTimeData.tm_mday != timeData.tm_mday)
	{
		return UnixtimeToDate(unixtime, "%d %B");
	}
	else
	{
		return UnixtimeToDate(unixtime, "%H:%M:%S");
	}
}

String format::CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric)
{
	for (size_t i = 0; i < dirtyString.size(); i++)
	{
		switch(dirtyString[i])
		{
		case '\b':
			if (color)
			{
				dirtyString.erase(i, 2);
				i--;
			}
			else
				i++;
			break;
		case '\x0E':
			if (color)
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		case '\x0F':
			if (color)
			{
				dirtyString.erase(i, 4);
				i--;
			}
			else
				i += 3;
			break;
		case '\r':
		case '\n':
			if (newlines)
				dirtyString[i] = ' ';
			break;
		default:
			if (numeric && (dirtyString[i] < '0' || dirtyString[i] > '9'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			// if less than ascii 20 or greater than ascii 126, delete
			else if (ascii && (dirtyString[i] < ' ' || dirtyString[i] > '~'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		}
	}
	return dirtyString;
}

std::vector<char> format::VideoBufferToPPM(const VideoBuffer & vidBuf)
{
	std::vector<char> data;
	char buffer[256];
	sprintf(buffer, "P6\n%d %d\n255\n", vidBuf.Width, vidBuf.Height);
	data.insert(data.end(), buffer, buffer+strlen(buffer));

	unsigned char * currentRow = new unsigned char[vidBuf.Width*3];
	for(int y = 0; y < vidBuf.Height; y++)
	{
		int rowPos = 0;
		for(int x = 0; x < vidBuf.Width; x++)
		{
			currentRow[rowPos++] = PIXR(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXG(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXB(vidBuf.Buffer[(y*vidBuf.Width)+x]);
		}
		data.insert(data.end(), currentRow, currentRow+(vidBuf.Width*3));
	}
	delete [] currentRow;

	return data;
}

const static char hex[] = "0123456789ABCDEF";

ByteString format::URLEncode(ByteString source)
{
	ByteString result;
	for (auto it = source.begin(); it < source.end(); ++it)
	{
		if (!((*it >= 'a' && *it <= 'z') ||
		      (*it >= 'A' && *it <= 'Z') ||
		      (*it >= '0' && *it <= '9')))
		{
			auto byte = uint8_t(*it);
			result.append(1, '%');
			result.append(1, hex[(byte >> 4) & 0xF]);
			result.append(1, hex[ byte       & 0xF]);
		}
		else
		{
			result.append(1, *it);
		}
	}
	return result;
}

ByteString format::URLDecode(ByteString source)
{
	ByteString result;
	for (auto it = source.begin(); it < source.end(); ++it)
	{
		if (*it == '%' && it < source.end() + 2)
		{
			auto byte = uint8_t(0);
			for (auto i = 0; i < 2; ++i)
			{
				it += 1;
				auto *off = strchr(hex, tolower(*it));
				if (!off)
				{
					return {};
				}
				byte = (byte << 4) | (off - hex);
			}
			result.append(1, byte);
		}
		else if (*it == '+')
		{
			result.append(1, ' ');
		}
		else
		{
			result.append(1, *it);
		}
	}
	return result;
}

void format::RenderTemperature(StringBuilder &sb, float temp, int scale)
{
	switch (scale)
	{
	case 1:
		sb << (temp - 273.15f) << "C";
		break;
	case 2:
		sb << (temp - 273.15f) * 1.8f + 32.0f << "F";
		break;
	default:
		sb << temp << "K";
		break;
	}
}

float format::StringToTemperature(String str, int defaultScale)
{
	auto scale = defaultScale;
	if (str.size())
	{
		if (str.EndsWith("K"))
		{
			scale = 0;
			str = str.SubstrFromEnd(1);
		}
		else if (str.EndsWith("C"))
		{
			scale = 1;
			str = str.SubstrFromEnd(1);
		}
		else if (str.EndsWith("F"))
		{
			scale = 2;
			str = str.SubstrFromEnd(1);
		}
	}
	if (!str.size())
	{
		throw std::out_of_range("empty string");
	}
	auto out = str.ToNumber<float>();
	switch (scale)
	{
	case 1:
		out = out + 273.15;
		break;
	case 2:
		out = (out - 32.0f) / 1.8f + 273.15f;
		break;
	}
	return out;
}
