#include "Platform.h"
#include "resource.h"
#include "Config.h"
#include <iostream>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#include <crtdbg.h>
#include <memory>

namespace Platform
{
ByteString GetCwd()
{
	ByteString cwd;
	auto cwdPtr = std::unique_ptr<wchar_t, decltype(&free)>(_wgetcwd(nullptr, 0), free);
	if (cwdPtr)
	{
		cwd = WinNarrow(cwdPtr.get());
	}
	return cwd;
}

void OpenURI(ByteString uri)
{
	if (int(INT_PTR(ShellExecuteW(nullptr, nullptr, WinWiden(uri).c_str(), nullptr, nullptr, SW_SHOWNORMAL))) <= 32)
	{
		fprintf(stderr, "cannot open URI: ShellExecute(...) failed\n");
	}
}

void Millisleep(long int t)
{
	Sleep(t);
}

long unsigned int GetTime()
{
	return GetTickCount();
}

bool Stat(ByteString filename)
{
	struct _stat s;
	if (_wstat(WinWiden(filename).c_str(), &s) == 0)
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
	struct _stat s;
	if (_wstat(WinWiden(filename).c_str(), &s) == 0)
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
	struct _stat s;
	if (_wstat(WinWiden(directory).c_str(), &s) == 0)
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
	struct _stat s;
	if (_wstat(WinWiden(path).c_str(), &s) == 0)
	{
		if (GetFileAttributesW(WinWiden(path).c_str()) & FILE_ATTRIBUTE_REPARSE_POINT)
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
	return _wremove(WinWiden(filename).c_str()) == 0;
}

bool RenameFile(ByteString filename, ByteString newFilename, bool replace)
{
	if (replace)
	{
		// TODO: we rely on errno but errors from this are available through GetLastError(); fix
		return MoveFileExW(WinWiden(filename).c_str(), WinWiden(newFilename).c_str(), MOVEFILE_REPLACE_EXISTING);
	}
	return _wrename(WinWiden(filename).c_str(), WinWiden(newFilename).c_str()) == 0;
}

bool DeleteDirectory(ByteString folder)
{
	return _wrmdir(WinWiden(folder).c_str()) == 0;
}

bool MakeDirectory(ByteString dir)
{
	return _wmkdir(WinWiden(dir).c_str()) == 0;
}

bool ChangeDir(ByteString toDir)
{
	return _wchdir(WinWiden(toDir).c_str()) == 0;
}

std::vector<ByteString> DirectoryList(ByteString directory)
{
	std::vector<ByteString> directoryList;
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
	return directoryList;
}

ByteString WinNarrow(const std::wstring &source)
{
	int buffer_size = WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), nullptr, 0, nullptr, nullptr);
	if (!buffer_size)
	{
		return "";
	}
	std::string output(buffer_size, 0);
	if (!WideCharToMultiByte(CP_UTF8, 0, source.c_str(), source.size(), output.data(), buffer_size, nullptr, nullptr))
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
	if (!MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), output.data(), buffer_size))
	{
		return L"";
	}
	return output;
}

ByteString ExecutableName()
{
	std::wstring buf(L"?");
	while (true)
	{
		SetLastError(ERROR_SUCCESS);
		if (!GetModuleFileNameW(nullptr, buf.data(), DWORD(buf.size())))
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
	return WinNarrow(buf.data()); // Pass pointer to copy only up to the zero terminator.
}

void DoRestart()
{
	ByteString exename = ExecutableName();
	if (exename.length())
	{
		int ret = int(INT_PTR(ShellExecuteW(nullptr, nullptr, WinWiden(exename).c_str(), nullptr, nullptr, SW_SHOWNORMAL)));
		if (ret <= 32)
		{
			fprintf(stderr, "cannot restart: ShellExecute(...) failed: code %i\n", ret);
		}
		else
		{
			Exit(0);
		}
	}
	else
	{
		fprintf(stderr, "cannot restart: no executable name???\n");
	}
	Exit(-1);
}

bool CanUpdate()
{
	return true;
}

bool Install()
{
	bool ok = true;
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
		ok = ok && RegCreateKeyExW(HKEY_CURRENT_USER, wPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &k, nullptr) == ERROR_SUCCESS;
		ok = ok && RegSetValueExW(k, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(wValue.c_str()), (wValue.size() + 1) * 2) == ERROR_SUCCESS;
		if (wExtraKey.size())
		{
			ok = ok && RegSetValueExW(k, wExtraKey.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(wExtraValue.c_str()), (wExtraValue.size() + 1) * 2) == ERROR_SUCCESS;
		}
		RegCloseKey(k);
		return ok;
	};

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
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
	IShellLinkW *shellLink = nullptr;
	IPersistFile *shellLinkPersist = nullptr;
	wchar_t programsPath[MAX_PATH];
	ok = ok && SHGetFolderPathW(nullptr, CSIDL_PROGRAMS, nullptr, SHGFP_TYPE_CURRENT, programsPath) == S_OK;
	ok = ok && CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID *)&shellLink) == S_OK;
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
	return ok;
}

bool UpdateStart(std::span<const char> data)
{
	ByteString exeName = Platform::ExecutableName(), updName;

	if (!exeName.length())
		return false;

	updName = exeName;
	ByteString extension = exeName.substr(exeName.length() - 4);
	if (extension == ".exe")
		updName = exeName.substr(0, exeName.length() - 4);
	updName = updName + "_upd.exe";

	if (!RenameFile(exeName, updName, false))
		return false;

	if (!WriteFile(data, exeName))
	{
		Platform::RemoveFile(exeName);
		return false;
	}

	if ((uintptr_t)ShellExecute(nullptr, L"open", Platform::WinWiden(exeName).c_str(), nullptr, nullptr, SW_SHOWNORMAL) <= 32)
	{
		Platform::RemoveFile(exeName);
		return false;
	}

	return true;
}

bool UpdateFinish()
{
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
	return true;
}

void UpdateCleanup()
{
	UpdateFinish();
}

void SetupCrt()
{
	_setmode(0, _O_BINARY);
	_setmode(1, _O_BINARY);
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	if constexpr (DEBUG)
	{
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	}
	if (GetACP() != CP_UTF8)
	{
		std::cerr << "failed to set codepage to utf-8, expect breakage" << std::endl;
	}
}
}
