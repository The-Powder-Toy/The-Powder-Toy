#pragma once

#include "common/String.h"
#include <vector>

class VideoBuffer;

namespace format
{
	const static char hex[] = "0123456789ABCDEF";

	template <typename T> ByteString NumberToByteString(T number)
	{
		ByteString::Stream ss;
		ss << number;
		return ss.str();
	}

	template <typename T> String NumberToString(T number)
	{
		String::Stream ss;
		ss << number;
		return ss.str();
	}

	template <typename T> T ByteStringToNumber(const ByteString & text)
	{
		ByteString::Stream ss(text);
		T number;
		return (ss >> number)?number:0;
	}

	template <typename T> T StringToNumber(const String & text)
	{
		String::Stream ss(text);
		T number;
		return (ss >> number)?number:0;
	}

	ByteString URLEncode(ByteString value);
	ByteString UnixtimeToDate(time_t unixtime, ByteString dateFomat = "%d %b %Y");
	ByteString UnixtimeToDateMini(time_t unixtime);
	String CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric = false);
	std::vector<char> VideoBufferToPNG(const VideoBuffer & vidBuf);
	std::vector<char> VideoBufferToBMP(const VideoBuffer & vidBuf);
	std::vector<char> VideoBufferToPPM(const VideoBuffer & vidBuf);
	std::vector<char> VideoBufferToPTI(const VideoBuffer & vidBuf);
	VideoBuffer * PTIToVideoBuffer(std::vector<char> & data);
	unsigned long CalculateCRC(unsigned char * data, int length);
}
