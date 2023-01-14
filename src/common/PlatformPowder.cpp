#include "Platform.h"
#include "client/Client.h"
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
# include <mach-o/dyld.h>
#endif
#include <fstream>

namespace Platform
{
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
			Client::Ref().Shutdown(); // very ugly hack; will fix soon(tm)
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
	auto screenshots = DirectorySearch(fromDir, "screenshot", { ".png" });
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

	if (scripts.size())
		Client::Ref().RescanStamps();

	logFile << std::endl << std::endl << "Migration complete. Results: " << result.Build().ToUtf8();
	logFile.close();

	return result.Build();
}
}
