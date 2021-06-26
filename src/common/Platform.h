#ifndef PLATFORM_H
#define PLATFORM_H
#include "Config.h"

#include "common/String.h"

#ifdef WIN
# include <string>
#endif

namespace Platform
{
	std::string GetCwd();
	ByteString ExecutableName();
	void DoRestart();

	void OpenURI(ByteString uri);

	void Millisleep(long int t);
	long unsigned int GetTime();

	void LoadFileInResource(int name, int type, unsigned int& size, const char*& data);

	bool Stat(std::string filename);
	bool FileExists(std::string filename);
	bool DirectoryExists(std::string directory);
	/**
	 * @return true on success
	 */
	bool DeleteFile(std::string filename);

	/**
	 * @return true on success
	 */
	bool DeleteDirectory(std::string folder);

	/**
	 * @return true on success
	 */
	bool MakeDirectory(std::string dir);
	std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions);

#ifdef WIN
	ByteString WinNarrow(const std::wstring &source);
	std::wstring WinWiden(const ByteString &source);
#endif
}

#endif
