#include "Platform.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#ifdef WIN
#define NOMINMAX
#include <direct.h>
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
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

std::string originalCwd;
std::string sharedCwd;

ByteString GetCwd()
{
	char *cwd = getcwd(NULL, 0);
	return cwd == nullptr ? "" : cwd;
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
	return std::remove(filename.c_str()) == 0;
}

bool DeleteDirectory(ByteString folder)
{
#ifdef WIN
	return _rmdir(folder.c_str()) == 0;
#else
	return rmdir(folder.c_str()) == 0;
#endif
}

bool MakeDirectory(ByteString dir)
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
		return std::vector<ByteString>();
	}
	do
	{
		ByteString currentFileName = Platform::WinNarrow(currentFile.name);
		if (currentFileName.length() > 4)
			directoryList.push_back(currentFileName);
	}
	while (_wfindnext(findFileHandle, &currentFile) == 0);
	_findclose(findFileHandle);
#else
	//Linux or MinGW
	struct dirent * directoryEntry;
	DIR *directoryHandle = opendir(directory.c_str());
	if (!directoryHandle)
	{
		return std::vector<ByteString>();
	}
	while ((directoryEntry = readdir(directoryHandle)))
	{
		ByteString currentFileName = ByteString(directoryEntry->d_name);
		if (currentFileName.length()>4)
			directoryList.push_back(currentFileName);
	}
	closedir(directoryHandle);
#endif

	search = search.ToLower();

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
				tempfilename = filename.SubstrFromEnd(0, (*extIter).size()).ToLower();
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

String DoMigration(ByteString fromDir, ByteString toDir)
{
	if (fromDir.at(fromDir.length() - 1) != '/')
		fromDir = fromDir + '/';
	if (toDir.at(toDir.length() - 1) != '/')
		toDir = toDir + '/';

	std::ofstream logFile(fromDir + "/migrationlog.txt", std::ios::out);
	logFile << "Running migration of data from " << fromDir + " to " << toDir << std::endl;

	// Get lists of files to migrate
	auto stamps = DirectorySearch(fromDir + "stamps", "", { ".stm" });
	auto saves = DirectorySearch(fromDir + "Saves", "", { ".cps", ".stm" });
	auto scripts = DirectorySearch(fromDir + "scripts", "", { ".lua", ".txt" });
	auto downloadedScripts = DirectorySearch(fromDir + "scripts/downloaded", "", { ".lua" });
	bool hasScriptinfo = FileExists(toDir + "scripts/downloaded/scriptinfo");
	auto screenshots = DirectorySearch(fromDir, "powdertoy-", { ".png" });
	bool hasAutorun = FileExists(fromDir + "autorun.lua");
	bool hasPref = FileExists(fromDir + "powder.pref");

	if (stamps.empty() && saves.empty() && scripts.empty() && downloadedScripts.empty() && screenshots.empty() && !hasAutorun && !hasPref)
	{
		logFile << "Nothing to migrate.";
		return "Nothing to migrate. This button is used to migrate data from pre-96.0 TPT installations to the shared directory";
	}

	StringBuilder result;
	std::stack<ByteString> dirsToDelete;

	// Migrate a list of files
	auto migrateList = [&](std::vector<ByteString> list, ByteString directory, String niceName) {
		result << '\n' << niceName << ": ";
		if (!list.empty() && !directory.empty())
			MakeDirectory(toDir + directory);
		int migratedCount = 0, failedCount = 0;
		for (auto &item : list)
		{
			std::string from = fromDir + directory + "/" + item;
			std::string to = toDir + directory + "/" + item;
			if (!FileExists(to))
			{
				if (rename(from.c_str(), to.c_str()))
				{
					failedCount++;
					logFile << "failed to move " << from << " to " << to << std::endl;
				}
				else
				{
					migratedCount++;
					logFile << "moved " << from << " to " << to << std::endl;
				}
			}
			else
			{
				logFile << "skipping " << from << "(already exists)" << std::endl;
			}
		}

		dirsToDelete.push(directory);
		result << "\bt" << migratedCount << " migratated\x0E, \br" << failedCount << " failed\x0E";
		int duplicates = list.size() - migratedCount - failedCount;
		if (duplicates)
			result << ", " << list.size() - migratedCount - failedCount << " skipped (duplicate)";
	};

	// Migrate a single file
	auto migrateFile = [&fromDir, &toDir, &result, &logFile](ByteString filename) {
		ByteString from = fromDir + filename;
		ByteString to = toDir + filename;
		if (!FileExists(to))
		{
			if (rename(from.c_str(), to.c_str()))
			{
				logFile << "failed to move " << from << " to " << to << std::endl;
				result << "\n\br" << filename.FromUtf8() << " migration failed\x0E";
			}
			else
			{
				logFile << "moved " << from << " to " << to << std::endl;
				result << '\n' << filename.FromUtf8() << " migrated";
			}
		}
		else
		{
			logFile << "skipping " << from << "(already exists)" << std::endl;
			result << '\n' << filename.FromUtf8() << " skipped (already exists)";
		}

		if (!RemoveFile(fromDir + filename)) {
			logFile << "failed to delete " << filename << std::endl;
		}
	};

	// Do actual migration
	RemoveFile(fromDir + "stamps/stamps.def");
	migrateList(stamps, "stamps", "Stamps");
	migrateList(saves, "Saves", "Saves");
	if (!scripts.empty())
		migrateList(scripts, "scripts", "Scripts");
	if (!hasScriptinfo && !downloadedScripts.empty())
	{
		migrateList(downloadedScripts, "scripts/downloaded", "Downloaded scripts");
		migrateFile("scripts/downloaded/scriptinfo");
	}
	if (!screenshots.empty())
		migrateList(screenshots, "", "Screenshots");
	if (hasAutorun)
		migrateFile("autorun.lua");
	if (hasPref)
		migrateFile("powder.pref");

	// Delete leftover directories
	while (!dirsToDelete.empty())
	{
		ByteString toDelete = dirsToDelete.top();
		if (!DeleteDirectory(fromDir + toDelete)) {
			logFile << "failed to delete " << toDelete << std::endl;
		}
		dirsToDelete.pop();
	}

	// chdir into the new directory
	chdir(toDir.c_str());

#if !defined(RENDERER) && !defined(FONTEDITOR)
	if (scripts.size())
		Client::Ref().RescanStamps();
#endif

	logFile << std::endl << std::endl << "Migration complete. Results: " << result.Build().ToUtf8();
	logFile.close();

	return result.Build();
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
