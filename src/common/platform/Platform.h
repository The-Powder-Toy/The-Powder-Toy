#pragma once
#include "common/String.h"
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <optional>

namespace Platform
{
	ByteString GetCwd();
	ByteString ExecutableNameFirstApprox();
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	bool Stat(ByteString filename);
	bool FileExists(ByteString filename);
	bool DirectoryExists(ByteString directory);
	bool IsLink(ByteString path);
	/**
	 * @return true on success
	 */
	bool RemoveFile(ByteString filename);
	bool RenameFile(ByteString filename, ByteString newFilename, bool replace);

	/**
	 * @return true on success
	 */
	bool DeleteDirectory(ByteString folder);

	/**
	 * @return true on success
	 */
	bool MakeDirectory(ByteString dir);
	std::vector<ByteString> DirectoryList(ByteString directory);
	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions);

	bool ReadFile(std::vector<char> &fileData, ByteString filename);
	bool WriteFile(std::span<const char> fileData, ByteString filename);

	// TODO: Remove these and switch to *A Win32 API variants when we stop fully supporting windows
	//       versions older than win10 1903, for example when win10 reaches EOL, see 18084d5aa0e5.
	ByteString WinNarrow(const std::wstring &source);
	std::wstring WinWiden(const ByteString &source);

	extern std::string originalCwd;
	extern std::string sharedCwd;

	bool CanUpdate();

	bool Install();

	bool ChangeDir(ByteString toDir);

	bool UpdateStart(std::span<const char> data);
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
