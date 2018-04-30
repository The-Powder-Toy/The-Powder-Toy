#ifndef PLATFORM_H
#define PLATFORM_H

#include "common/String.h"

namespace Platform
{
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();
}

#endif
