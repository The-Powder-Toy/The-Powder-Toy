#pragma once

#include <sstream>
#include <vector>

class VideoBuffer;

namespace format
{
	static char hex[] = "0123456789ABCDEF";

	template <typename T> std::string NumberToString(T number)
	{
		std::stringstream ss;
		ss << number;
		return ss.str();
	}

	template <typename T> T StringToNumber(const std::string & text)
	{
		std::stringstream ss(text);
		T number;
		return (ss >> number)?number:0;
	}

	std::string URLEncode(std::string value);
	std::string UnixtimeToDate(time_t unixtime, std::string dateFomat = "%d %b %Y");
	std::string UnixtimeToDateMini(time_t unixtime);
	std::string CleanString(std::string dirtyString, int maxVisualSize, int maxStringLength);
	std::string CleanString(std::string dirtyString, int maxStringLength = std::string::npos);
	std::string CleanString(char * dirtyData, int maxVisualSize, int maxStringLength);
	std::string CleanString(char * dirtyData, int maxStringLength);
	std::vector<char> VideoBufferToPNG(const VideoBuffer & vidBuf);
	std::vector<char> VideoBufferToPPM(const VideoBuffer & vidBuf);
	std::vector<char> VideoBufferToPTI(const VideoBuffer & vidBuf);
	VideoBuffer * PTIToVideoBuffer(std::vector<char> & data);
	unsigned long CalculateCRC(unsigned char * data, int length);
}