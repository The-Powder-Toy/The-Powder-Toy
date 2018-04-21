#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>

namespace Platform
{
	std::string ExecutableName();
	void DoRestart();

	void OpenURI(std::string uri);

	void Millisleep(long int t);
	long unsigned int GetTime();
}

#endif
