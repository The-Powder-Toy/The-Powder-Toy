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
ByteString GetCwd()
{
	ByteString cwd;
	char *cwdPtr = getcwd(NULL, 0);
	if (cwdPtr)
	{
		cwd = cwdPtr;
	}
	free(cwdPtr);
	return cwd;
}

void Millisleep(long int t)
{
	struct timespec s;
	s.tv_sec = t / 1000;
	s.tv_nsec = (t % 1000) * 10000000;
	nanosleep(&s, NULL);
}

bool Stat(ByteString filename)
{
	struct stat s;
	if (stat(filename.c_str(), &s) == 0)
	{
		return true; // Something exists, be it a file, directory, link, etc.
	}
	else
	{
		return false; // Doesn't exist
	}
}

bool FileExists(ByteString filename)
{
	struct stat s;
	if (stat(filename.c_str(), &s) == 0)
	{
		if(s.st_mode & S_IFREG)
		{
			return true; // Is file
		}
		else
		{
			return false; // Is directory or something else
		}
	}
	else
	{
		return false; // Doesn't exist
	}
}

bool DirectoryExists(ByteString directory)
{
	struct stat s;
	if (stat(directory.c_str(), &s) == 0)
	{
		if(s.st_mode & S_IFDIR)
		{
			return true; // Is directory
		}
		else
		{
			return false; // Is file or something else
		}
	}
	else
	{
		return false; // Doesn't exist
	}
}

bool IsLink(ByteString path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFLNK)
		{
			return true; // Is path
		}
		else
		{
			return false; // Is file or something else
		}
	}
	else
	{
		return false; // Doesn't exist
	}
}

bool RemoveFile(ByteString filename)
{
	return remove(filename.c_str()) == 0;
}

bool RenameFile(ByteString filename, ByteString newFilename, bool replace)
{
	// TODO: Make atomic :( Could use renameat2 with RENAME_NOREPLACE on linux and
	// renamex_np with RENAME_EXCL on darwin, but both require filesystem support;
	// I don't think it's worth it for now. -- LBPHacker
	if (!replace && FileExists(newFilename))
	{
		return false;
	}
	return rename(filename.c_str(), newFilename.c_str()) == 0;
}

bool DeleteDirectory(ByteString folder)
{
	return rmdir(folder.c_str()) == 0;
}

bool MakeDirectory(ByteString dir)
{
	return mkdir(dir.c_str(), 0755) == 0;
}

bool ChangeDir(ByteString toDir)
{
	return chdir(toDir.c_str()) == 0;
}

std::vector<ByteString> DirectoryList(ByteString directory)
{
	std::vector<ByteString> directoryList;
	struct dirent * directoryEntry;
	DIR *directoryHandle = opendir(directory.c_str());
	if (!directoryHandle)
	{
		return std::vector<ByteString>();
	}
	while ((directoryEntry = readdir(directoryHandle)))
	{
		directoryList.push_back(ByteString(directoryEntry->d_name));
	}
	closedir(directoryHandle);
	return directoryList;
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

void SetupCrt()
{
}
}
