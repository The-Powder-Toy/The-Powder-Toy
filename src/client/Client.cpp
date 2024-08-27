#include "Client.h"
#include "prefs/GlobalPrefs.h"
#include "client/http/StartupRequest.h"
#include "ClientListener.h"
#include "Format.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/UserInfo.h"
#include "common/platform/Platform.h"
#include "common/String.h"
#include "graphics/Graphics.h"
#include "gui/dialogues/ErrorMessage.h"
#include "prefs/Prefs.h"
#include "lua/CommandInterface.h"
#include "Config.h"
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <set>

Client::Client():
	messageOfTheDay("Fetching the message of the day..."),
	usingAltUpdateServer(false),
	updateAvailable(false),
	authUser(0, "")
{
	LoadAuthUser();
	auto &prefs = GlobalPrefs::Ref();
	firstRun = !prefs.BackedByFile();
}

void Client::MigrateStampsDef()
{
	std::vector<char> data;
	if (!Platform::ReadFile(data, ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, "stamps.def")))
	{
		return;
	}
	for (auto i = 0; i < int(data.size()); i += 10)
	{
		stampIDs.push_back(ByteString(data.data() + i, data.data() + i + 10));
	}
}

void Client::Initialize()
{
	auto &prefs = GlobalPrefs::Ref();
	if (prefs.Get("version.update", false))
	{
		prefs.Set("version.update", false);
		Platform::UpdateFinish();
	}

	stamps = std::make_unique<Prefs>(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, "stamps.json"));
	stampIDs = stamps->Get("MostRecentlyUsedFirst", std::vector<ByteString>{});
	{
		Prefs::DeferWrite dw(*stamps);
		if (!stamps->BackedByFile())
		{
			MigrateStampsDef();
			WriteStamps();
		}
		RescanStamps();
	}

	//Begin version check
	versionCheckRequest = std::make_unique<http::StartupRequest>(false);
	versionCheckRequest->Start();
	if constexpr (USE_UPDATESERVER)
	{
		// use an alternate update server
		alternateVersionCheckRequest = std::make_unique<http::StartupRequest>(true);
		alternateVersionCheckRequest->Start();
		usingAltUpdateServer = true;
	}
}

bool Client::IsFirstRun()
{
	return firstRun;
}

void Client::SetMessageOfTheDay(String message)
{
	messageOfTheDay = message;
	notifyMessageOfTheDay();
}

String Client::GetMessageOfTheDay()
{
	return messageOfTheDay;
}

void Client::AddServerNotification(ServerNotification notification)
{
	serverNotifications.push_back(notification);
	notifyNewNotification(notification);
}

std::vector<ServerNotification> Client::GetServerNotifications()
{
	return serverNotifications;
}

void Client::Tick()
{
	auto applyUpdateInfo = false;
	if (versionCheckRequest && versionCheckRequest->CheckDone())
	{
		if (versionCheckRequest->StatusCode() == 618)
		{
			AddServerNotification({ "Failed to load SSL certificates", ByteString::Build(SERVER, "/FAQ.html") });
		}
		try
		{
			auto info = versionCheckRequest->Finish();
			if (!info.sessionGood)
			{
				SetAuthUser(User(0, ""));
			}
			if (!usingAltUpdateServer)
			{
				updateInfo = info.updateInfo;
				applyUpdateInfo = true;
				SetMessageOfTheDay(info.messageOfTheDay);
			}
			for (auto &notification : info.notifications)
			{
				AddServerNotification(notification);
			}
		}
		catch (const http::RequestError &ex)
		{
			if (!usingAltUpdateServer)
			{
				SetMessageOfTheDay(ByteString::Build("Error while fetching MotD: ", ex.what()).FromUtf8());
			}
		}
		versionCheckRequest.reset();
	}
	if (alternateVersionCheckRequest && alternateVersionCheckRequest->CheckDone())
	{
		try
		{
			auto info = alternateVersionCheckRequest->Finish();
			updateInfo = info.updateInfo;
			applyUpdateInfo = true;
			SetMessageOfTheDay(info.messageOfTheDay);
			for (auto &notification : info.notifications)
			{
				AddServerNotification(notification);
			}
		}
		catch (const http::RequestError &ex)
		{
			SetMessageOfTheDay(ByteString::Build("Error while checking for updates: ", ex.what()).FromUtf8());
		}
		alternateVersionCheckRequest.reset();
	}
	if (applyUpdateInfo && !IGNORE_UPDATES)
	{
		if (updateInfo)
		{
			notifyUpdateAvailable();
		}
	}
}

std::optional<UpdateInfo> Client::GetUpdateInfo()
{
	return updateInfo;
}

void Client::notifyUpdateAvailable()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyUpdateAvailable(this);
	}
}

void Client::notifyMessageOfTheDay()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyMessageOfTheDay(this);
	}
}

void Client::notifyAuthUserChanged()
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyAuthUserChanged(this);
	}
}

void Client::notifyNewNotification(ServerNotification notification)
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		(*iterator)->NotifyNewNotification(this, notification);
	}
}

void Client::AddListener(ClientListener * listener)
{
	listeners.push_back(listener);
}

void Client::RemoveListener(ClientListener * listener)
{
	for (std::vector<ClientListener*>::iterator iterator = listeners.begin(), end = listeners.end(); iterator != end; ++iterator)
	{
		if((*iterator) == listener)
		{
			listeners.erase(iterator);
			return;
		}
	}
}

Client::~Client()
{
}

void Client::SetAuthUser(User user)
{
	authUser = user;
	SaveAuthUser();
	notifyAuthUserChanged();
}

User Client::GetAuthUser()
{
	return authUser;
}

void Client::MoveStampToFront(ByteString stampID)
{
	auto it = std::find(stampIDs.begin(), stampIDs.end(), stampID);
	auto changed = false;
	if (it == stampIDs.end())
	{
		stampIDs.push_back(stampID);
		it = stampIDs.end() - 1;
		changed = true;
	}
	else if (it != stampIDs.begin())
	{
		changed = true;
	}
	if (changed)
	{
		std::rotate(stampIDs.begin(), it, it + 1);
		WriteStamps();
	}
}

std::unique_ptr<SaveFile> Client::GetStamp(ByteString stampID)
{
	ByteString stampFile = ByteString(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, stampID, ".stm"));
	auto saveFile = LoadSaveFile(stampFile);
	if (!saveFile)
		saveFile = LoadSaveFile(stampID);
	else
		saveFile->SetDisplayName(stampID.FromUtf8());
	return saveFile;
}

void Client::DeleteStamp(ByteString stampID)
{
	auto it = std::remove(stampIDs.begin(), stampIDs.end(), stampID);
	if (it != stampIDs.end())
	{
		stampIDs.erase(it, stampIDs.end());
		Platform::RemoveFile(ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, stampID, ".stm"));
		WriteStamps();
	}
}

void Client::RenameStamp(ByteString stampID, ByteString newName)
{
	auto oldPath = ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, stampID, ".stm");
	auto newPath = ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, newName, ".stm");

	if (Platform::FileExists(newPath))
	{
		new ErrorMessage("Error renaming stamp", "A stamp with this name already exists.");
		return;
	}

	if (!Platform::RenameFile(oldPath, newPath, false))
	{
		new ErrorMessage("Error renaming stamp", "Could not rename the stamp.");
		return;
	}

	std::replace(stampIDs.begin(), stampIDs.end(), stampID, newName);
	WriteStamps();
}

ByteString Client::AddStamp(std::unique_ptr<GameSave> saveData)
{
	auto now = (uint64_t)time(NULL);
	if (lastStampTime != now)
	{
		lastStampTime = now;
		lastStampName = 0;
	}
	else
	{
		lastStampName += 1;
	}
	ByteString saveID, filename;
	while (true)
	{
		saveID = ByteString::Build(Format::Hex(Format::Width(lastStampTime, 8)), Format::Hex(Format::Width(lastStampName, 2)));
		filename = ByteString::Build(STAMPS_DIR, PATH_SEP_CHAR, saveID, ".stm");
		if (!Platform::FileExists(filename))
		{
			break;
		}
		lastStampName += 1;
	}

	Platform::MakeDirectory(STAMPS_DIR);

	Json::Value stampInfo;
	stampInfo["type"] = "stamp";
	stampInfo["username"] = authUser.Username;
	stampInfo["name"] = filename;
	stampInfo["date"] = Json::Value::UInt64(now);
	if (authors.size() != 0)
	{
		// This is a stamp, always append full authorship info (even if same user)
		stampInfo["links"].append(Client::Ref().authors);
	}
	saveData->authors = stampInfo;

	std::vector<char> gameData;
	std::tie(std::ignore, gameData) = saveData->Serialise();
	if (!gameData.size())
		return "";

	Platform::WriteFile(gameData, filename);
	MoveStampToFront(saveID);
	return saveID;
}

void Client::RescanStamps()
{
	ByteString extension = ".stm";
	std::set<ByteString> stampFilesSet;
	for (auto &stampID : Platform::DirectorySearch("stamps", "", { extension }))
	{
		stampFilesSet.insert(stampID.substr(0, stampID.size() - extension.size()));
	}
	std::vector<ByteString> newStampIDs;
	auto changed = false;
	for (auto &stampID : stampIDs)
	{
		if (stampFilesSet.find(stampID) == stampFilesSet.end())
		{
			changed = true;
		}
		else
		{
			newStampIDs.push_back(stampID);
		}
	}
	auto stampIDsSet = std::set<ByteString>(stampIDs.begin(), stampIDs.end());
	for (auto &stampID : stampFilesSet)
	{
		if (stampIDsSet.find(stampID) == stampIDsSet.end())
		{
			newStampIDs.push_back(stampID);
			changed = true;
		}
	}
	if (changed)
	{
		stampIDs = newStampIDs;
		WriteStamps();
	}
}

void Client::WriteStamps()
{
	if (stampIDs.size())
	{
		stamps->Set("MostRecentlyUsedFirst", stampIDs);
	}
}

const std::vector<ByteString> &Client::GetStamps() const
{
	return stampIDs;
}

std::unique_ptr<SaveFile> Client::LoadSaveFile(ByteString filename)
{
	ByteString err;
	std::unique_ptr<SaveFile> file;
	if (Platform::FileExists(filename))
	{
		file = std::make_unique<SaveFile>(filename);
		try
		{
			std::vector<char> data;
			if (Platform::ReadFile(data, filename))
			{
				file->SetGameSave(std::make_unique<GameSave>(std::move(data)));
			}
			else
			{
				err = "failed to open";
			}
		}
		catch (const ParseException &e)
		{
			err = e.what();
		}
	}
	else
	{
		err = "does not exist";
	}
	if (err.size())
	{
		std::cerr << "Client: " << filename << ": " << err << std::endl;
		if (file)
		{
			file->SetLoadingError(err.FromUtf8());
		}
	}
	return file;
}

// stamp-specific wrapper for MergeAuthorInfo
// also used for clipboard and lua stamps
void Client::MergeStampAuthorInfo(Json::Value stampAuthors)
{
	if (stampAuthors.size())
	{
		// when loading stamp/clipboard, only append info to authorship info (since we aren't replacing the save)
		// unless there is nothing loaded currently, then set authors directly
		if (authors.size())
		{
			// Don't add if it's exactly the same
			if (stampAuthors["links"].size() == 1 && stampAuthors["links"][0] == Client::Ref().authors)
				return;
			if (authors["username"] != stampAuthors["username"])
			{
				// 2nd arg of MergeAuthorInfo needs to be an array
				Json::Value toAdd;
				toAdd.append(stampAuthors);
				MergeAuthorInfo(toAdd);
			}
			else if (stampAuthors["links"].size())
			{
				MergeAuthorInfo(stampAuthors["links"]);
			}
		}
		else
			authors = stampAuthors;
	}
}

// linksToAdd is an array (NOT an object) of links to add to authors["links"]
void Client::MergeAuthorInfo(Json::Value linksToAdd)
{
	for (Json::Value::ArrayIndex i = 0; i < linksToAdd.size(); i++)
	{
		// link is the same exact json we have open, don't do anything
		if (linksToAdd[i] == authors)
			return;

		bool hasLink = false;
		for (Json::Value::ArrayIndex j = 0; j < authors["links"].size(); j++)
		{
			// check everything in authors["links"] to see if it's the same json as what we are already adding
			if (authors["links"][j] == linksToAdd[i])
				hasLink = true;
		}
		if (!hasLink)
			authors["links"].append(linksToAdd[i]);
	}
}

// load current authors information into a json value (when saving everything: stamps, clipboard, local saves, and online saves)
void Client::SaveAuthorInfo(Json::Value *saveInto)
{
	if (authors.size() != 0)
	{
		// Different username? Save full original save info
		if (authors["username"] != (*saveInto)["username"])
			(*saveInto)["links"].append(authors);
		// This is probalby the same save
		// Don't append another layer of links, just keep existing links
		else if (authors["links"].size())
			(*saveInto)["links"] = authors["links"];
	}
}

bool AddCustomGol(String ruleString, String nameString, unsigned int highColor, unsigned int lowColor)
{
	auto &prefs = GlobalPrefs::Ref();
	auto customGOLTypes = prefs.Get("CustomGOL.Types", std::vector<ByteString>{});
	std::vector<ByteString> newCustomGOLTypes;
	bool nameTaken = false;
	for (auto gol : customGOLTypes)
	{
		auto parts = gol.FromUtf8().PartitionBy(' ');
		if (parts.size())
		{
			if (parts[0] == nameString)
			{
				nameTaken = true;
			}
		}
		newCustomGOLTypes.push_back(gol);
	}
	if (nameTaken)
		return false;

	StringBuilder sb;
	sb << nameString << " " << ruleString << " " << highColor << " " << lowColor;
	newCustomGOLTypes.push_back(sb.Build().ToUtf8());
	prefs.Set("CustomGOL.Types", newCustomGOLTypes);
	return true;
}

String Client::DoMigration(ByteString fromDir, ByteString toDir)
{
	if (fromDir.at(fromDir.length() - 1) != '/')
		fromDir = fromDir + '/';
	if (toDir.at(toDir.length() - 1) != '/')
		toDir = toDir + '/';

	std::ofstream logFile(fromDir + "/migrationlog.txt", std::ios::out);
	logFile << "Running migration of data from " << fromDir + " to " << toDir << std::endl;

	// Get lists of files to migrate
	auto stamps = Platform::DirectorySearch(fromDir + "stamps", "", { ".stm" });
	auto saves = Platform::DirectorySearch(fromDir + "Saves", "", { ".cps", ".stm" });
	auto scripts = Platform::DirectorySearch(fromDir + "scripts", "", { ".lua", ".txt" });
	auto downloadedScripts = Platform::DirectorySearch(fromDir + "scripts/downloaded", "", { ".lua" });
	bool hasScriptinfo = Platform::FileExists(toDir + "scripts/downloaded/scriptinfo");
	auto screenshots = Platform::DirectorySearch(fromDir, "screenshot", { ".png" });
	bool hasAutorun = Platform::FileExists(fromDir + "autorun.lua");
	bool hasPref = Platform::FileExists(fromDir + "powder.pref");

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
			Platform::MakeDirectory(toDir + directory);
		int migratedCount = 0, failedCount = 0;
		for (auto &item : list)
		{
			std::string from = fromDir + directory + "/" + item;
			std::string to = toDir + directory + "/" + item;
			if (!Platform::FileExists(to))
			{
				if (Platform::RenameFile(from, to, false))
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
		if (!Platform::FileExists(to))
		{
			if (Platform::RenameFile(from, to, false))
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

		if (!Platform::RemoveFile(fromDir + filename)) {
			logFile << "failed to delete " << filename << std::endl;
		}
	};

	// Do actual migration
	Platform::RemoveFile(fromDir + "stamps/stamps.def");
	Platform::RemoveFile(fromDir + "stamps/stamps.json");
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
		if (!Platform::DeleteDirectory(fromDir + toDelete)) {
			logFile << "failed to delete " << toDelete << std::endl;
		}
		dirsToDelete.pop();
	}

	// chdir into the new directory
	Platform::ChangeDir(toDir);

	RescanStamps();

	logFile << std::endl << std::endl << "Migration complete. Results: " << result.Build().ToUtf8();
	logFile.close();

	return result.Build();
}
