#ifndef PLATFORM_H
#define PLATFORM_H
#include "Config.h"

#include "common/String.h"

#ifdef WIN
# include <string>
#endif

namespace Platform
{
	ByteString GetCwd();
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	void LoadFileInResource(int name, int type, unsigned int& size, const char*& data);

	bool Stat(ByteString filename);
	bool FileExists(ByteString filename);
	bool DirectoryExists(ByteString directory);
	/**
	 * @return true on success
	 */
	bool RemoveFile(ByteString filename);

	/**
	 * @return true on success
	 */
	bool DeleteDirectory(ByteString folder);

	/**
	 * @return true on success
	 */
	bool MakeDirectory(ByteString dir);
	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions);
	String DoMigration(ByteString fromDir, ByteString toDir);

#ifdef WIN
	ByteString WinNarrow(const std::wstring &source);
	std::wstring WinWiden(const ByteString &source);
#endif

	extern std::string originalCwd;
	extern std::string sharedCwd;
}

#endif
