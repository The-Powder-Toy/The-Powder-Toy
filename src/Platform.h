#ifndef PLATFORM_H
#define PLATFORM_H
#include "Config.h"

#include "common/String.h"

#ifdef WIN
# include <string>
#endif

namespace Platform
{
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	void LoadFileInResource(int name, int type, unsigned int& size, const char*& data);

#ifdef WIN
	ByteString WinNarrow(const std::wstring &source);
	std::wstring WinWiden(const ByteString &source);
#endif
}

#endif
