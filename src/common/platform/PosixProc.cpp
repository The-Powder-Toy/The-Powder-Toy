#include "Platform.h"
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include <dirent.h>

namespace Platform
{
void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
		execl(exename.c_str(), exename.c_str(), NULL);
		int ret = errno;
		fprintf(stderr, "cannot restart: execl(...) failed: code %i\n", ret);
	}
	else
	{
		fprintf(stderr, "cannot restart: no executable name???\n");
	}
	Exit(-1);
}

ByteString ExecutableName()
{
	auto firstApproximation = ExecutableNameFirstApprox();
	auto rp = std::unique_ptr<char, decltype(std::free) *>(realpath(&firstApproximation[0], NULL), std::free);
	if (!rp)
	{
		std::cerr << "realpath: " << errno << std::endl;
		return "";
	}
	return rp.get();
}

bool UpdateStart(const std::vector<char> &data)
{
	ByteString exeName = Platform::ExecutableName();

	if (!exeName.length())
		return false;

	auto updName = exeName + "-update";

	if (!WriteFile(data, updName))
	{
		RemoveFile(updName);
		return false;
	}

	if (chmod(updName.c_str(), 0755))
	{
		RemoveFile(updName);
		return false;
	}

	if (!RenameFile(updName, exeName, true))
	{
		RemoveFile(updName);
		return false;
	}

	execl(exeName.c_str(), "powder-update", NULL);
	return false; // execl returned, we failed
}

bool UpdateFinish()
{
	return true;
}

void UpdateCleanup()
{
}
}
