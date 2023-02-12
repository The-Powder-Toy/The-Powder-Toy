#include "Platform.h"
#include "resource.h"
#include "common/tpt-rand.h"
#include "Config.h"
#include <memory>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace Platform
{

std::string originalCwd;
std::string sharedCwd;

// Returns a list of all files in a directory matching a search
// search - list of search terms. extensions - list of extensions to also match
std::vector<ByteString> DirectorySearch(ByteString directory, ByteString search, std::vector<ByteString> extensions)
{
	//Get full file listing
	//Normalise directory string, ensure / or \ is present
	if (!directory.size() || (directory.back() != '/' && directory.back() != '\\'))
		directory.append(1, PATH_SEP_CHAR);
	auto directoryList = DirectoryList(directory);

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
			writeFileName = ByteString::Build(filename, ".temp.", Format::Width(5), Format::Fill('0'), interfaceRng() % 100000);
			if (!FileExists(writeFileName))
			{
				break;
			}
		}
	}
	bool ok = false;
	{
		std::ofstream f(writeFileName, std::ios::binary);
		if (f) f.write(&fileData[0], fileData.size());
		ok = bool(f);
	}
	if (!ok)
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
}
