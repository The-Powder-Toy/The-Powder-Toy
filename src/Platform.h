#ifndef PLATFORM_H
#define PLATFORM_H

#include "common/String.h"

namespace Platform
{
	ByteString PrefFileDirectory();
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	void LoadFileInResource(int name, int type, unsigned int& size, const char*& data);

	int MakeDirectory(const char * dirname);
	int MakeDirectory(const ByteString &dirname);
	int MakeDirectoryChain(const char * dirname);
	int MakeDirectoryChain(ByteString dirname);
}

#endif
