#include "Platform.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#ifdef WIN
#define NOMINMAX
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <ctime>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#endif

#include "Misc.h"

namespace Platform
{

ByteString PrefFileDirectory()
{
#if defined(WIN)
	char *appdata = getenv("APPDATA");
	if(appdata != nullptr) return ByteString(appdata) + "\\The Powder Toy\\";
	char *homedrive = getenv("HOMEDRIVE");
	char *homepath = getenv("HOMEPATH");
	if(homedrive != nullptr && homepath != nullptr) return ByteString(homedrive) + homepath + "\\AppData\\Roaming\\The Powder Toy\\";
#elif defined(LIN)
	char *xdg_data = getenv("XDG_DATA_HOME");
	if(xdg_data != nullptr) return ByteString(xdg_data) + "/powdertoy/";
	char *home = getenv("HOME");
	if(home != nullptr) return ByteString(home) + "/.local/share/powdertoy/";
	char *username = getlogin();
	if(username != nullptr) return "/home/" + ByteString(username) + "/.local/share/powdertoy/";
#elif defined(MACOSX)
	FSRef ref;
	OSType folderType = kApplicationSupportFolderType;
	char path[PATH_MAX];
	FSFindFolder( kUserDomain, folderType, kCreateFolder, &ref );
	FSRefMakePath( &ref, (UInt8*)&path, PATH_MAX );
	return ByteString(path) + "/The Powder Toy";
#endif
	return "";
}

ByteString ExecutableName()
{
	ByteString ret;
#if defined(WIN)
	char *name = (char *)malloc(64);
	DWORD max = 64, res;
	while ((res = GetModuleFileName(NULL, name, max)) >= max)
	{
#elif defined MACOSX
	char *fn = (char*)malloc(64),*name = (char*)malloc(PATH_MAX);
	uint32_t max = 64, res;
	if (_NSGetExecutablePath(fn, &max) != 0)
	{
		char *realloced_fn = (char*)realloc(fn, max);
		assert(realloced_fn != NULL);
		fn = realloced_fn;
		_NSGetExecutablePath(fn, &max);
	}
	if (realpath(fn, name) == NULL)
	{
		free(fn);
		free(name);
		return "";
	}
	res = 1;
#else
	char fn[64], *name = (char *)malloc(64);
	size_t max = 64, res;
	sprintf(fn, "/proc/self/exe");
	memset(name, 0, max);
	while ((res = readlink(fn, name, max)) >= max-1)
	{
#endif
#ifndef MACOSX
		max *= 2;
		char* realloced_name = (char *)realloc(name, max);
		assert(realloced_name != NULL);
		name = realloced_name;
		memset(name, 0, max);
	}
#endif
	if (res <= 0)
	{
		free(name);
		return "";
	}
	ret = name;
	free(name);
	return ret;
}

void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
#ifdef WIN
		ShellExecute(NULL, "open", exename.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(LIN) || defined(MACOSX)
		execl(exename.c_str(), "powder", NULL);
#endif
	}
	exit(-1);
}

void OpenURI(ByteString uri)
{
#if defined(WIN)
	ShellExecute(0, "OPEN", uri.c_str(), NULL, NULL, 0);
#elif defined(MACOSX)
	char *cmd = (char*)malloc(7+uri.length());
	strcpy(cmd, "open ");
	strappend(cmd, (char*)uri.c_str());
	system(cmd);
#elif defined(LIN)
	char *cmd = (char*)malloc(11+uri.length());
	strcpy(cmd, "xdg-open ");
	strappend(cmd, (char*)uri.c_str());
	system(cmd);
#else
	printf("Cannot open browser\n");
#endif
}

void Millisleep(long int t)
{
#ifdef WIN
	Sleep(t);
#else
	struct timespec s;
	s.tv_sec = t / 1000;
	s.tv_nsec = (t % 1000) * 10000000;
	nanosleep(&s, NULL);
#endif
}

long unsigned int GetTime()
{
#ifdef WIN
	return GetTickCount();
#elif defined(MACOSX)
	struct timeval s;
	gettimeofday(&s, NULL);
	return (unsigned int)(s.tv_sec * 1000 + s.tv_usec / 1000);
#else
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
#endif
}


void LoadFileInResource(int name, int type, unsigned int& size, const char*& data)
{
#ifdef _MSC_VER
	HMODULE handle = ::GetModuleHandle(NULL);
	HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
	HGLOBAL rcData = ::LoadResource(handle, rc);
	size = ::SizeofResource(handle, rc);
	data = static_cast<const char*>(::LockResource(rcData));
#endif
}

int MakeDirectory(ByteString const &dirName)
{
#ifdef WIN
	return _mkdir(dirName.c_str());
#else
	return mkdir(dirName.c_str(), 0755);
#endif
}

int MakeDirectoryChain(ByteString dirName)
{
	int success = MakeDirectory(dirName);
	if(success == 0) return 0;

	// If the directory already exists, consider it a success
	if(errno == EEXIST) return 0;

	// ENOENT signifies that an element of the path does not exist, i.e. a parent directory is missing.
	// If the error is ENOENT, descend in path and try to create parent directories.
	// If it's something else, bail out and return failure.
	if(errno != ENOENT) return -1;

	unsigned long slashPos = dirName.find_last_of(PATH_SEP_CHAR, 0);

	// No slash, means we can't descent further in path
	if(slashPos == dirName.npos) return -1;

	// Descend in path and try again
	dirName.Erase(slashPos, dirName.length());
	return MakeDirectoryChain(dirName);
}

}
