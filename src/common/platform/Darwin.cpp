#include "Platform.h"
#include <iostream>
#include <sys/time.h>
#include <cstdint>
#include <mach-o/dyld.h>

namespace Platform
{

void OpenURI(ByteString uri)
{
	if (system(("open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
}

long unsigned int GetTime()
{
	struct timeval s;
	gettimeofday(&s, NULL);
	return (unsigned int)(s.tv_sec * 1000 + s.tv_usec / 1000);
}

ByteString ExecutableNameFirstApprox()
{
	ByteString firstApproximation("?");
	{
		auto bufSize = uint32_t(firstApproximation.size());
		auto ret = _NSGetExecutablePath(firstApproximation.data(), &bufSize);
		if (ret == -1)
		{
			// Buffer not large enough; likely to happen since it's initially a single byte.
			firstApproximation.resize(bufSize);
			ret = _NSGetExecutablePath(firstApproximation.data(), &bufSize);
		}
		if (ret != 0)
		{
			// Can't even get a first approximation.
			std::cerr << "_NSGetExecutablePath: " << ret << std::endl;
			return "";
		}
	}
	return firstApproximation;
}

bool CanUpdate()
{
	return false;
}

void SetupCrt()
{
}
}
