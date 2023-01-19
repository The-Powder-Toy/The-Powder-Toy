#include "Platform.h"
#include "resource.h"
#include "tpt-rand.h"
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
# include <ApplicationServices/ApplicationServices.h>
#endif
#ifdef LIN
# include "icon_cps.png.h"
# include "icon_exe.png.h"
# include "save.xml.h"
# include "powder.desktop.h"
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

bool RenameFile(ByteString filename, ByteString newFilename, bool replace)
{
#ifdef WIN
	if (replace)
	{
		// TODO: we rely on errno but errors from this are available through GetLastError(); fix
		return MoveFileExW(WinWiden(filename).c_str(), WinWiden(newFilename).c_str(), MOVEFILE_REPLACE_EXISTING);
	}
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

bool ChangeDir(ByteString toDir)
{
#ifdef WIN
	return _wchdir(WinWiden(toDir).c_str()) == 0;
#else
	return chdir(toDir.c_str()) == 0;
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

bool WriteFile(const std::vector<char> &fileData, ByteString filename)
{
	auto replace = FileExists(filename);
	auto writeFileName = filename;
	if (replace)
	{
		while (true)
		{
			writeFileName = ByteString::Build(filename, ".temp.", random_gen() % 100000);
			if (!FileExists(writeFileName))
			{
				break;
			}
		}
	}
	std::ofstream f(writeFileName, std::ios::binary);
	if (f)
	{
		f.write(&fileData[0], fileData.size());
	}
	if (!f)
	{
		std::cerr << "WriteFile: " << filename << ": " << strerror(errno) << std::endl;
		if (replace)
		{
			RemoveFile(writeFileName);
		}
		return false;
	}
	if (replace)
	{
		if (!RenameFile(writeFileName, filename, true))
		{
			RemoveFile(writeFileName);
			return false;
		}
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

bool CanUpdate()
{
#ifdef MACOSX
	return false;
#else
	return true;
#endif
}

bool CanInstall()
{
	return INSTALL_CHECK;
}

bool Install()
{
	bool ok = true;
#if defined(WIN)
	auto deleteKey = [](ByteString path) {
		RegDeleteKeyW(HKEY_CURRENT_USER, Platform::WinWiden(path).c_str());
	};
	auto createKey = [](ByteString path, ByteString value, ByteString extraKey = {}, ByteString extraValue = {}) {
		auto ok = true;
		auto wPath = Platform::WinWiden(path);
		auto wValue = Platform::WinWiden(value);
		auto wExtraKey = Platform::WinWiden(extraKey);
		auto wExtraValue = Platform::WinWiden(extraValue);
		HKEY k;
		ok = ok && RegCreateKeyExW(HKEY_CURRENT_USER, wPath.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &k, NULL) == ERROR_SUCCESS;
		ok = ok && RegSetValueExW(k, NULL, 0, REG_SZ, reinterpret_cast<const BYTE *>(wValue.c_str()), (wValue.size() + 1) * 2) == ERROR_SUCCESS;
		if (wExtraKey.size())
		{
			ok = ok && RegSetValueExW(k, wExtraKey.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(wExtraValue.c_str()), (wExtraValue.size() + 1) * 2) == ERROR_SUCCESS;
		}
		RegCloseKey(k);
		return ok;
	};

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	auto exe = Platform::ExecutableName();
#ifndef IDI_DOC_ICON
	// make this fail so I don't remove #include "resource.h" again and get away with it
# error where muh IDI_DOC_ICON D:
#endif
	auto icon = ByteString::Build(exe, ",-", IDI_DOC_ICON);
	auto path = Platform::GetCwd();
	auto open = ByteString::Build("\"", exe, "\" ddir \"", path, "\" \"file://%1\"");
	auto ptsave = ByteString::Build("\"", exe, "\" ddir \"", path, "\" \"%1\"");
	deleteKey("Software\\Classes\\ptsave");
	deleteKey("Software\\Classes\\.cps");
	deleteKey("Software\\Classes\\.stm");
	deleteKey("Software\\Classes\\PowderToySave");
	ok = ok && createKey("Software\\Classes\\ptsave", "Powder Toy Save", "URL Protocol", "");
	ok = ok && createKey("Software\\Classes\\ptsave\\DefaultIcon", icon);
	ok = ok && createKey("Software\\Classes\\ptsave\\shell\\open\\command", ptsave);
	ok = ok && createKey("Software\\Classes\\.cps", "PowderToySave");
	ok = ok && createKey("Software\\Classes\\.stm", "PowderToySave");
	ok = ok && createKey("Software\\Classes\\PowderToySave", "Powder Toy Save");
	ok = ok && createKey("Software\\Classes\\PowderToySave\\DefaultIcon", icon);
	ok = ok && createKey("Software\\Classes\\PowderToySave\\shell\\open\\command", open);
	IShellLinkW *shellLink = NULL;
	IPersistFile *shellLinkPersist = NULL;
	wchar_t programsPath[MAX_PATH];
	ok = ok && SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, programsPath) == S_OK;
	ok = ok && CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID *)&shellLink) == S_OK;
	ok = ok && shellLink->SetPath(Platform::WinWiden(exe).c_str()) == S_OK;
	ok = ok && shellLink->SetWorkingDirectory(Platform::WinWiden(path).c_str()) == S_OK;
	ok = ok && shellLink->SetDescription(Platform::WinWiden(APPNAME).c_str()) == S_OK;
	ok = ok && shellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&shellLinkPersist) == S_OK;
	ok = ok && shellLinkPersist->Save(Platform::WinWiden(ByteString::Build(Platform::WinNarrow(programsPath), "\\", APPNAME, ".lnk")).c_str(), TRUE) == S_OK;
	if (shellLinkPersist)
	{
		shellLinkPersist->Release();
	}
	if (shellLink)
	{
		shellLink->Release();
	}
	CoUninitialize();
#elif defined(LIN)
	auto desktopEscapeString = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			auto from = " " "\n" "\t" "\r" "\\";
			auto to   = "s"  "n"  "t"  "r" "\\";
			if (auto off = strchr(from, ch))
			{
				escaped.append(1, '\\');
				escaped.append(1, to[off - from]);
			}
			else
			{
				escaped.append(1, ch);
			}
		}
		return escaped;
	};
	auto desktopEscapeExec = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			if (strchr(" \t\n\"\'\\><~|&;$*?#()`", ch))
			{
				escaped.append(1, '\\');
			}
			escaped.append(1, ch);
		}
		return escaped;
	};

	if (ok)
	{
		ByteString desktopData(powder_desktop, powder_desktop + powder_desktop_size);
		auto exe = Platform::ExecutableName();
		auto path = exe.SplitFromEndBy('/').Before();
		desktopData = desktopData.Substitute("Exec=" + ByteString(APPEXE), "Exec=" + desktopEscapeString(desktopEscapeExec(exe)));
		desktopData += ByteString::Build("Path=", desktopEscapeString(path), "\n");
		ByteString file = ByteString::Build(APPVENDOR, "-", APPID, ".desktop");
		ok = ok && Platform::WriteFile(std::vector<char>(desktopData.begin(), desktopData.end()), file);
		ok = ok && !system(ByteString::Build("xdg-desktop-menu install ", file).c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " application/vnd.powdertoy.save").c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " x-scheme-handler/ptsave").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-save.xml";
		ok = ok && Platform::WriteFile(std::vector<char>(save_xml, save_xml + save_xml_size), file);
		ok = ok && !system(ByteString::Build("xdg-mime install ", file).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-cps.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_cps_png, icon_cps_png + icon_cps_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --context mimetypes --size 64 ", file, " application-vnd.powdertoy.save").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-exe.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_exe_png, icon_exe_png + icon_exe_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --size 64 ", file, " ", APPVENDOR, "-", APPEXE).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ok = ok && !system("xdg-icon-resource forceupdate");
	}
#else
	ok = false;
#endif
	return ok;
}

bool UpdateStart(const std::vector<char> &data)
{
	ByteString exeName = Platform::ExecutableName(), updName;

	if (!exeName.length())
		return false;

#ifdef WIN
	updName = exeName;
	ByteString extension = exeName.substr(exeName.length() - 4);
	if (extension == ".exe")
		updName = exeName.substr(0, exeName.length() - 4);
	updName = updName + "_upd.exe";

	if (!MoveFile(Platform::WinWiden(exeName).c_str(), Platform::WinWiden(updName).c_str()))
		return false;

	if (!WriteFile(data, exeName))
	{
		Platform::RemoveFile(exeName);
		return false;
	}

	if ((uintptr_t)ShellExecute(NULL, L"open", Platform::WinWiden(exeName).c_str(), NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		Platform::RemoveFile(exeName);
		return false;
	}

	return true;
#else
	updName = exeName + "-update";

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

	if (!RenameFile(updName, exeName))
	{
		RemoveFile(updName);
		return false;
	}

	execl(exeName.c_str(), "powder-update", NULL);
	return false; // execl returned, we failed
#endif
}

bool UpdateFinish()
{
#ifdef WIN
	ByteString exeName = Platform::ExecutableName(), updName;
	int timeout = 5, err;
	if constexpr (DEBUG)
	{
		printf("Update: Current EXE name: %s\n", exeName.c_str());
	}
	updName = exeName;
	ByteString extension = exeName.substr(exeName.length() - 4);
	if (extension == ".exe")
		updName = exeName.substr(0, exeName.length() - 4);
	updName = updName + "_upd.exe";
	if constexpr (DEBUG)
	{
		printf("Update: Temp EXE name: %s\n", updName.c_str());
	}
	while (!Platform::RemoveFile(updName))
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
		{
			if constexpr (DEBUG)
			{
				printf("Update: Temp file not deleted\n");
			}
			// Old versions of powder toy name their update files with _update.exe, delete that upgrade file here
			updName = exeName;
			ByteString extension = exeName.substr(exeName.length() - 4);
			if (extension == ".exe")
				updName = exeName.substr(0, exeName.length() - 4);
			updName = updName + "_update.exe";
			Platform::RemoveFile(updName);
			return true;
		}
		Sleep(500);
		timeout--;
		if (timeout <= 0)
		{
			if constexpr (DEBUG)
			{
				printf("Update: Delete timeout\n");
			}
			return false;
		}
	}
#endif
	return true;
}

void UpdateCleanup()
{
#ifdef WIN
	UpdateFinish();
#endif
}
}
