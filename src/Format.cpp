
#include <time.h>
#include <string>
#include "Format.h"

std::string format::UnixtimeToDate(time_t unixtime, std::string dateFormat)
{
	struct tm * timeData;
	char buffer[128];

	timeData = localtime(&unixtime);

	strftime(buffer, 128, dateFormat.c_str(), timeData);
	return std::string(buffer);
}

std::string format::UnixtimeToDateMini(time_t unixtime)
{
	time_t currentTime = time(NULL);
	struct tm currentTimeData = *localtime(&currentTime);
	struct tm timeData = *localtime(&unixtime);

	if(currentTimeData.tm_year != timeData.tm_year)
	{
		return UnixtimeToDate(unixtime, "%b %Y");
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
