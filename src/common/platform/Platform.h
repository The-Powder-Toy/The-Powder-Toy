#pragma once
#include "common/String.h"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace Platform
{
	ByteString GetCwd();
	ByteString ExecutableNameFirstApprox();
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(const ByteString& uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	bool Stat(const ByteString& filename);
	bool FileExists(const ByteString& filename);
	bool DirectoryExists(const ByteString& directory);
	bool IsLink(const ByteString& path);
	/**
	 * @return true on success
	 */
	bool RemoveFile(const ByteString& filename);
	bool RenameFile(const ByteString& filename, const ByteString& newFilename, bool replace);

	/**
	 * @return true on success
	 */
	bool DeleteDirectory(const ByteString& folder);

	/**
	 * @return true on success
	 */
	bool MakeDirectory(const ByteString& dir);
	std::vector<ByteString> DirectoryList(const ByteString& directory);
	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, const std::vector<ByteString>& extensions);

	bool ReadFile(std::vector<char> &fileData, const ByteString& filename);
	bool WriteFile(const std::vector<char> &fileData, const ByteString& filename);

	// TODO: Remove these and switch to *A Win32 API variants when we stop fully supporting windows
	//       versions older than win10 1903, for example when win10 reaches EOL, see 18084d5aa0e5.
	ByteString WinNarrow(const std::wstring &source);
	std::wstring WinWiden(const ByteString &source);

	extern std::string originalCwd;
	extern std::string sharedCwd;

	bool CanUpdate();

	bool Install();

	bool ChangeDir(const ByteString& toDir);

	bool UpdateStart(const std::vector<char> &data);
	bool UpdateFinish();
	void UpdateCleanup();

	void SetupCrt();

	using ExitFunc = void (*)();
	void Atexit(ExitFunc exitFunc);
	void Exit(int code);

	ByteString DefaultDdir();

	int InvokeMain(int argc, char *argv[]);

	std::optional<std::vector<String>> StackTrace();

	void MarkPresentable();
}

extern "C" int Main(int argc, char *argv[]);
