#include "Platform.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <dirent.h>
#include <sys/stat.h>

#ifdef WIN
#define NOMINMAX
#include <direct.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h>
#endif

#include "Misc.h"
#include "client/Client.h"

namespace Platform
{

std::string GetCwd()
{
	char cwdTemp[PATH_MAX];
	getcwd(cwdTemp, PATH_MAX);
	return cwdTemp;
}

ByteString ExecutableName()
{
	ByteString ret;
#if defined(WIN)
	using Char = wchar_t;
#elif defined(LIN)
	using Char = char;
#endif
#if defined(WIN)
	wchar_t *name = (wchar_t *)malloc(sizeof(wchar_t) * 64);
	DWORD max = 64, res;
	while ((res = GetModuleFileNameW(NULL, name, max)) >= max)
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
		Char* realloced_name = (Char *)realloc(name, sizeof(Char) * max);
		assert(realloced_name != NULL);
		name = realloced_name;
		memset(name, 0, sizeof(Char) * max);
	}
#endif
	if (res <= 0)
	{
		free(name);
		return "";
	}
#if defined(WIN)
	ret = WinNarrow(name);
#else
	ret = name;
#endif
	free(name);
	return ret;
}

void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
#ifdef WIN
		int ret = int(INT_PTR(ShellExecuteW(NULL, NULL, WinWiden(exename).c_str(), NULL, NULL, SW_SHOWNORMAL)));
		if (ret <= 32)
		{
			fprintf(stderr, "cannot restart: ShellExecute(...) failed: code %i\n", ret);
		}
		else
		{
#if !defined(RENDERER) && !defined(FONTEDITOR)
			Client::Ref().Shutdown(); // very ugly hack; will fix soon(tm)
#endif
			exit(0);
		}
#elif defined(LIN) || defined(MACOSX)
		execl(exename.c_str(), "powder", NULL);
		int ret = errno;
		fprintf(stderr, "cannot restart: execl(...) failed: code %i\n", ret);
#endif
	}
	else
	{
		fprintf(stderr, "cannot restart: no executable name???\n");
	}
	exit(-1);
}

void OpenURI(ByteString uri)
{
#if defined(WIN)
	if (int(INT_PTR(ShellExecuteW(NULL, NULL, WinWiden(uri).c_str(), NULL, NULL, SW_SHOWNORMAL))) <= 32)
	{
		fprintf(stderr, "cannot open URI: ShellExecute(...) failed\n");
	}
#elif defined(MACOSX)
	if (system(("open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
#elif defined(LIN)
	if (system(("xdg-open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
#else
	fprintf(stderr, "cannot open URI: not implemented\n");
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

bool Stat(std::string filename)
{
#ifdef WIN
	struct _stat s;
	if (_stat(filename.c_str(), &s) == 0)
#else
	struct stat s;
	if (stat(filename.c_str(), &s) == 0)
#endif
	{
		return true; // Something exists, be it a file, directory, link, etc.
	}
	else
	{
		return false; // Doesn't exist
	}
}

bool FileExists(std::string filename)
{
#ifdef WIN
	struct _stat s;
	if (_stat(filename.c_str(), &s) == 0)
#else
	struct stat s;
	if (stat(filename.c_str(), &s) == 0)
#endif
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

bool DirectoryExists(std::string directory)
{
#ifdef WIN
	struct _stat s;
	if (_stat(directory.c_str(), &s) == 0)
#else
	struct stat s;
	if (stat(directory.c_str(), &s) == 0)
#endif
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

bool DeleteFile(std::string filename)
{
	return std::remove(filename.c_str()) == 0;
}

bool DeleteDirectory(std::string folder)
{
#ifdef WIN
	return _rmdir(folder.c_str()) == 0;
#else
	return rmdir(folder.c_str()) == 0;
#endif
}

bool MakeDirectory(std::string dir)
{
#ifdef WIN
	return _mkdir(dir.c_str()) == 0;
#else
	return mkdir(dir.c_str(), 0755) == 0;
#endif
}

// Returns a list of all files in a directory matching a search
// search - list of search terms. extensions - list of extensions to also match
std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions)
{
	//Get full file listing
	//Normalise directory string, ensure / or \ is present
	if (*directory.rbegin() != '/' && *directory.rbegin() != '\\')
		directory += PATH_SEP;
	std::vector<ByteString> directoryList;
#if defined(WIN) && !defined(__GNUC__)
	//Windows
	struct _wfinddata_t currentFile;
	intptr_t findFileHandle;
	ByteString fileMatch = directory + "*.*";
	findFileHandle = _wfindfirst(Platform::WinWiden(fileMatch).c_str(), &currentFile);
	if (findFileHandle == -1L)
	{
#ifdef DEBUG
		printf("Unable to open directory: %s\n", directory.c_str());
#endif
		return std::vector<ByteString>();
	}
	do
	{
		ByteString currentFileName = Platform::WinNarrow(currentFile.name);
		if (currentFileName.length()>4)
			directoryList.push_back(directory+currentFileName);
	}
	while (_wfindnext(findFileHandle, &currentFile) == 0);
	_findclose(findFileHandle);
#else
	//Linux or MinGW
	struct dirent * directoryEntry;
	DIR *directoryHandle = opendir(directory.c_str());
	if (!directoryHandle)
	{
#ifdef DEBUG
		printf("Unable to open directory: %s\n", directory.c_str());
#endif
		return std::vector<ByteString>();
	}
	while ((directoryEntry = readdir(directoryHandle)))
	{
		ByteString currentFileName = ByteString(directoryEntry->d_name);
		if (currentFileName.length()>4)
			directoryList.push_back(directory+currentFileName);
	}
	closedir(directoryHandle);
#endif

	std::vector<ByteString> searchResults;
	for (std::vector<ByteString>::iterator iter = directoryList.begin(), end = directoryList.end(); iter != end; ++iter)
	{
		ByteString filename = *iter, tempfilename = *iter;
		bool extensionMatch = !extensions.size();
		for (std::vector<ByteString>::iterator extIter = extensions.begin(), extEnd = extensions.end(); extIter != extEnd; ++extIter)
		{
			if (filename.EndsWith(*extIter))
			{
				extensionMatch = true;
				tempfilename = filename.SubstrFromEnd(0, (*extIter).size()).ToUpper();
				break;
			}
		}
		bool searchMatch = !search.size();
		if (search.size() && tempfilename.Contains(search))
			searchMatch = true;

		if (searchMatch && extensionMatch)
			searchResults.push_back(filename);
	}

	//Filter results
	return searchResults;
}


#ifdef WIN
ByteString WinNarrow(const std::wstring &source)
{
	int buffer_size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), nullptr, 0, NULL, NULL);
	if (!buffer_size)
	{
		return "";
	}
	std::string output(buffer_size, 0);
	if (!WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), &output[0], buffer_size, NULL, NULL))
	{
		return "";
	}
	return output;
}

std::wstring WinWiden(const ByteString &source)
{
	int buffer_size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), nullptr, 0);
	if (!buffer_size)
	{
		return L"";
	}
	std::wstring output(buffer_size, 0);
	if (!MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), &output[0], buffer_size))
	{
		return L"";
	}
	return output;
}
#endif

}

#ifdef WIN
# undef main // thank you sdl
int main(int argc, char *argv[]);
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int argc;
	wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::vector<ByteString> argv;
	for (auto i = 0; i < argc; ++i)
	{
		argv.push_back(Platform::WinNarrow(std::wstring(wargv[i])));
	}
	std::vector<char *> argp;
	for (auto &arg : argv)
	{
		argp.push_back(&arg[0]);
	}
	return main(argc, &argp[0]);
}
#endif
