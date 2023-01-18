#include "Platform.h"
#include <memory>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#ifdef WIN
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <direct.h>
# include <io.h>
# include <shlobj.h>
# include <shlwapi.h>
# include <shellapi.h>
# include <windows.h>
#else
# include <unistd.h>
# include <ctime>
# include <sys/time.h>
# include <dirent.h>
#endif
#ifdef MACOSX
# include <cstdint>
# include <mach-o/dyld.h>
#endif

namespace Platform
{

std::string originalCwd;
std::string sharedCwd;

ByteString GetCwd()
{
	ByteString cwd;
#if defined(WIN)
	wchar_t *cwdPtr = _wgetcwd(NULL, 0);
	if (cwdPtr)
	{
		cwd = WinNarrow(cwdPtr);
	}
	free(cwdPtr);
#else
	char *cwdPtr = getcwd(NULL, 0);
	if (cwdPtr)
	{
		cwd = cwdPtr;
	}
	free(cwdPtr);
#endif
	return cwd;
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

bool Stat(ByteString filename)
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

bool FileExists(ByteString filename)
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

bool DirectoryExists(ByteString directory)
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

bool RemoveFile(ByteString filename)
{
#ifdef WIN
	return _wremove(WinWiden(filename).c_str()) == 0;
#else
	return remove(filename.c_str()) == 0;
#endif
}

bool RenameFile(ByteString filename, ByteString newFilename)
{
#ifdef WIN
	return _wrename(WinWiden(filename).c_str(), WinWiden(newFilename).c_str()) == 0;
#else
	return rename(filename.c_str(), newFilename.c_str()) == 0;
#endif
}

bool DeleteDirectory(ByteString folder)
{
#ifdef WIN
	return _wrmdir(WinWiden(folder).c_str()) == 0;
#else
	return rmdir(folder.c_str()) == 0;
#endif
}

bool MakeDirectory(ByteString dir)
{
#ifdef WIN
	return _wmkdir(WinWiden(dir).c_str()) == 0;
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
	if (!directory.size() || (directory.back() != '/' && directory.back() != '\\'))
		directory += PATH_SEP;
	std::vector<ByteString> directoryList;
#ifdef WIN
	struct _wfinddata_t currentFile;
	intptr_t findFileHandle;
	ByteString fileMatch = directory + "*.*";
	findFileHandle = _wfindfirst(Platform::WinWiden(fileMatch).c_str(), &currentFile);
	if (findFileHandle == -1L)
	{
		return std::vector<ByteString>();
	}
	do
	{
		directoryList.push_back(Platform::WinNarrow(currentFile.name));
	}
	while (_wfindnext(findFileHandle, &currentFile) == 0);
	_findclose(findFileHandle);
#else
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
#endif

	search = search.ToLower();

	std::vector<ByteString> searchResults;
	for (std::vector<ByteString>::iterator iter = directoryList.begin(), end = directoryList.end(); iter != end; ++iter)
	{
		ByteString filename = *iter, tempfilename = *iter;
		bool extensionMatch = !extensions.size();
		for (auto &extension : extensions)
		{
			if (filename.size() >= extension.size() && filename.EndsWith(extension))
			{
				extensionMatch = true;
				tempfilename = filename.SubstrFromEnd(0, extension.size()).ToLower();
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

bool ReadFile(std::vector<char> &fileData, ByteString filename)
{
	std::ifstream f(filename, std::ios::binary);
	if (f) f.seekg(0, std::ios::end);
	if (f) fileData.resize(f.tellg());
	if (f) f.seekg(0);
	if (f) f.read(&fileData[0], fileData.size());
	if (!f)
	{
		std::cerr << "ReadFile: " << filename << ": " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

bool WriteFile(std::vector<char> fileData, ByteString filename, bool replaceAtomically)
{
	// TODO: replaceAtomically
	std::ofstream f(filename, std::ios::binary);
	if (f) f.write(&fileData[0], fileData.size());
	if (!f)
	{
		std::cerr << "WriteFile: " << filename << ": " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

ByteString ExecutableName()
{
#ifdef WIN
	std::wstring buf(L"?");
	while (true)
	{
		SetLastError(ERROR_SUCCESS);
		if (!GetModuleFileNameW(NULL, &buf[0], DWORD(buf.size())))
		{
			std::cerr << "GetModuleFileNameW: " << GetLastError() << std::endl;
			return "";
		}
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			break;
		}
		buf.resize(buf.size() * 2);
	}
	return WinNarrow(&buf[0]); // Pass pointer to copy only up to the zero terminator.
#else
# ifdef MACOSX
	ByteString firstApproximation("?");
	{
		auto bufSize = uint32_t(firstApproximation.size());
		auto ret = _NSGetExecutablePath(&firstApproximation[0], &bufSize);
		if (ret == -1)
		{
			// Buffer not large enough; likely to happen since it's initially a single byte.
			firstApproximation.resize(bufSize);
			ret = _NSGetExecutablePath(&firstApproximation[0], &bufSize);
		}
		if (ret != 0)
		{
			// Can't even get a first approximation.
			std::cerr << "_NSGetExecutablePath: " << ret << std::endl;
			return "";
		}
	}
# else
	ByteString firstApproximation("/proc/self/exe");
# endif
	auto rp = std::unique_ptr<char, decltype(std::free) *>(realpath(&firstApproximation[0], NULL), std::free);
	if (!rp)
	{
		std::cerr << "realpath: " << errno << std::endl;
		return "";
	}
	return rp.get();
#endif
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
			exit(0);
		}
#elif defined(LIN) || defined(MACOSX)
		execl(exename.c_str(), exename.c_str(), NULL);
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
}
